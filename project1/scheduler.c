#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>

#include "scheduler.h"


int cmp_FIFO_SJF(const void* a, const void* b){
	int tmp = ((Process *)a)->ready_time - ((Process *)b)->ready_time;
	if(tmp == 0)
		tmp = ((Process *)a)->exec_time - ((Process *)b)->exec_time;
	return tmp;
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

static inline int wake_up(int pid){
    struct sched_param param;
    param.sched_priority = 0;
    return sched_setscheduler(pid, SCHED_OTHER, &param);
}

static inline int block_down(int pid){
    struct sched_param param;
    param.sched_priority = 0;
    return sched_setscheduler(pid, SCHED_IDLE, &param);	/* SCHED_IDLE: with very low priority*/
}


static inline int decide_proc(int policy, int N, Process* procs, int last_id, int* rr){
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
				int start_id = (curr_id + 1) % N;
				curr_id = -1;
				for(int i = 0; i < N; i++){
					// Find the nearest one in circular array from curr_id 
					if(procs[(start_id + i) % N].pid != -1){
						curr_id = (start_id + i) % N;
						break;
					}
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
	qsort(procs, N, sizeof(Process), cmp_FIFO_SJF);

/* Assign scheduler to CPU different from processes */
	assert(assign_cpu(getpid(), P_CPU) != -1);
	assert(wake_up(getpid()) >= 0);

/* Start */
	int last_id = -1;
	// last_id == -1 means there is no runner in the last round
	int curr_time = 0;
	int rr = TQ;
	int done_count = 0;
	while(1){
	/* Fork the process who's ready */
		for(int i = 0; i < N; i++)
			if(procs[i].ready_time == curr_time){
			//Execute Process
				procs[i].pid = exec_proc(procs[i].exec_time);
				assert(procs[i].pid != -1);
			// Block the process to wait for scheduling
				assert(block_down(procs[i].pid) >= 0);
			// Assign process to CPU different from scheduler
				assert(assign_cpu(procs[i].pid, C_CPU) != -1);
				printf("%s %d\n", procs[i].name, procs[i].pid);
				fflush(stdout);
			}
	
	/* Determine who's next */
		int curr_id = decide_proc(policy, N, procs, last_id, &rr);
		// curr_id == -1 means there'll be no runner in this round
#ifdef DEBUG_CURR
		printf("time = %d, curr_proc = %s\n", curr_time, procs[curr_id].name);
		fflush(stdout);
#endif
	/* Context Switch */
		if(curr_id != -1 && curr_id != last_id){
			if(last_id != -1 && procs[last_id].pid != -1)
				assert(block_down(procs[last_id].pid) >= 0);
			if(curr_id != -1 && procs[curr_id].pid != -1)
				assert(wake_up(procs[curr_id].pid) >= 0);
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
}

