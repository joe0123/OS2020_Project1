#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <signal.h>

#include "scheduler.h"
#include "process.h"


int cmp_FIFO(const void* a, const void* b){
	return ((Process *)a)->ready_time - ((Process *)b)->ready_time;
}

static inline int assign_cpu(int pid, int core){
	if (core > sizeof(cpu_set_t))
		return -1;

	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(core, &mask);

	if (sched_setaffinity(pid, sizeof(mask), &mask) < 0)
		return -1;

	return 0;
}

static inline int wake_up(int pid, int priority){
    struct sched_param param;
    param.sched_priority = priority;
    return sched_setscheduler(pid, SCHED_RR | SCHED_RESET_ON_FORK, &param);
}


static inline int block_down(int pid){
    struct sched_param param;
    param.sched_priority = 10;
    return sched_setscheduler(pid, SCHED_RR, &param);
}

static inline int swap_queue(int* queue, int queue_num){
	int tmp[queue_num];
	for(int i = 0; i < queue_num; i++)
		tmp[i] = queue[i];
	queue[queue_num - 1] = queue[0];
	for(int i = 1; i < queue_num; i++)
		queue[i - 1] = tmp[i];
}


static inline int decide_proc(int policy, int N, Process* procs, int last_id, int* rr, int* rr_queue, int rr_num){
	int curr_id = last_id;	// default: last runner is the next runner

	// (curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1)) means our curr_id is now absent.
	switch(policy){
		case FIFO:
			if(curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1)){
				int i = curr_id + 1;
				curr_id = -1;
				for(; i < N; i++)
					// Find the nearest one behind curr_id thanks to qsort
					if(procs[i].pid != -1){
						curr_id = i;
						break;
					}
			}
			break;
		case RR:
			if(curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1) || *rr == 0){
				if(rr_num <= 0)
					curr_id = -1;
				else{
					if(curr_id != -1 && procs[curr_id].pid != -1 && *rr == 0)
						swap_queue(rr_queue, rr_num);
					curr_id = rr_queue[0];
				}
				*rr = TQ;
			}
			break;
		case SJF:
			if(curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1)){
				curr_id = -1;
				for(int i = 0; i < N; i++){
					if(procs[i].pid == -1)
						continue;
					// Find one with shortest exec_time in the whole array
					if(curr_id == -1 || (curr_id != -1 && procs[i].exec_time < procs[curr_id].exec_time))
						curr_id = i;
				}
			}
			break;
		case PSJF:
			if(curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1))
				curr_id = -1;
			// Find one with shortest exec_time in the whole array regardless of curr_id
			for(int i = 0; i < N; i++){
				if(procs[i].pid == -1)
					continue;
				if(curr_id == -1 || (curr_id != -1 && procs[i].exec_time < procs[curr_id].exec_time))
					curr_id = i;
			}
			break;
	}
	return curr_id;
}


int scheduling(int policy, int N, Process *procs){
/* Sort the processes, key1=ready_time, key2=exec_time */
	qsort(procs, N, sizeof(Process), cmp_FIFO);

/* Assign scheduler to CPU different from processes */
	assert(assign_cpu(getpid(), P_CPU) != -1);
	assert(wake_up(getpid(), 80) >= 0);

/* Set a guard on child_CPU */
	pid_t guard_pid = fork();
	assert(guard_pid >= 0);
	if(guard_pid == 0)
		while(1);
	assert(assign_cpu(guard_pid, C_CPU) != -1);
	assert(wake_up(guard_pid, 50) >= 0);


/* Start */
	int last_id = -1;
	// last_id == -1 means there is no runner in the last round
	int curr_time = 0;
	int rr = TQ;
	int rr_queue[N];
	int rr_num = 0;
	int done_count = 0;
	while(1){
	/* Fork the process who's ready */
		for(int i = last_id + 1; i < N; i++){
			if(procs[i].ready_time > curr_time)
				break;
			else if(procs[i].ready_time == curr_time){
			// Execute Process	
				pid_t pid = fork();
				assert(pid >= 0);
				if(pid == 0)
					exec_proc(procs[i].exec_time);
				procs[i].pid = pid;
			// Block the process to wait for scheduling
				assert(block_down(procs[i].pid) >= 0);
			// Assign process to CPU different from scheduler
				assert(assign_cpu(procs[i].pid, C_CPU) != -1);
			// Print Name
				printf("%s %d\n", procs[i].name, procs[i].pid);
				fflush(stdout);
			// Append to RR_queue
				if(policy == RR){
					rr_queue[rr_num] = i;
					rr_num++;
				}
			}
		}
	
	/* Determine who's next */
		int curr_id = decide_proc(policy, N, procs, last_id, &rr, rr_queue, rr_num);
		// curr_id == -1 means there'll be no runner in this round
#ifdef DEBUG_CURR
		printf("time = %d, curr_proc = %s\n", curr_time, procs[curr_id].name);
		fflush(stdout);
#endif
	/* Context Switch */
		if(curr_id != last_id){
			if(curr_id != -1 && procs[curr_id].pid != -1)
				assert(wake_up(procs[curr_id].pid, 80) >= 0);
			if(last_id != -1 && procs[last_id].pid != -1)
				assert(block_down(procs[last_id].pid) >= 0);
#ifdef DEBUG_PRIORITY
			printf("curr_time = %d\n", curr_time);
			for(int i = 0; i < N; i++)
				if(procs[i].pid != -1)
					printf("%d ", sched_getscheduler(procs[i].pid));
				else
					printf("-1 ");
			printf("\n");
			fflush(stdout);
#endif
		}

	/* Run time */
		UNIT_TIME;
		curr_time += 1;
		if(curr_id != -1){
			procs[curr_id].exec_time--;
			rr--;
		}

	/* Wait for done process if exists */
		if(curr_id != -1 && procs[curr_id].exec_time <= 0){
			waitpid(procs[curr_id].pid, NULL, 0);
			procs[curr_id].pid = -1;
			if(policy == RR){
				swap_queue(rr_queue, rr_num);
				rr_num--;
			}
#ifdef DEBUG_DONE
			printf("Process %s is done at %d\n", procs[curr_id].name, curr_time);
			fflush(stdout);
#endif
			done_count += 1;
		/* End of scheduling */
			if(done_count == N)
				break;
		}
		if(curr_id != -1)
			last_id = curr_id;
	}
	kill(guard_pid, SIGKILL);
	waitpid(guard_pid, NULL, 0);
	exit(0);
}
