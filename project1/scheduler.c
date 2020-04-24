#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>

#include "scheduler.h"

#define DEBUG_DONE

int cmp_FIFO_SJF(const void* a, const void* b){
	int tmp = ((Process *)a)->ready_time - ((Process *)b)->ready_time;
	if(tmp == 0)
		tmp = ((Process *)a)->exec_time - ((Process *)b)->exec_time;
	return tmp;
}


int assign_cpu(int pid, int core){
	if (core > sizeof(cpu_set_t))
		return -1;

	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(core, &mask);

	if (sched_setaffinity(pid, sizeof(mask), &mask) < 0)
		return -1;

	return 0;
}

int wake_up(int pid){
    struct sched_param param;
    param.sched_priority = 0;
    return sched_setscheduler(pid, SCHED_OTHER, &param);	/* SCHED_OTHER: Standard Round-Robin*/
}

int block_down(int pid){
    struct sched_param param;
    param.sched_priority = 0;
    return sched_setscheduler(pid, SCHED_IDLE, &param);	/* SCHED_IDLE: with very low priority*/
}


int decide_proc(int policy, int N, Process* procs, int last_id, int* rr){
	int curr_id = last_id;
	
	switch(policy){
		case FIFO:
			if(curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1)){
				int i = curr_id + 1;
				curr_id = -1;
				for(; i < N; i++)
					if(procs[i].pid != -1){
						curr_id = i;
						break;
					}
			}
			break;
		case RR:
			if(curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1) || *rr == 0){
				int start_id;
				if(curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1))
					start_id = 0;
				else
					start_id = (curr_id + 1) % N;
				curr_id = -1;
				for(int i = 0; i < N; i++){
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
					if(curr_id == -1 || (curr_id != -1 && procs[i].exec_time < procs[curr_id].exec_time))
						curr_id = i;
				}
			}
			break;
		case PSJF:
			if(curr_id == -1 || (curr_id != -1 && procs[curr_id].pid == -1))
				curr_id = -1;
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
#ifdef DEBUG
	printf("Start Scheduling...\n");
	fflush(stdout);
#endif

	qsort(procs, N, sizeof(Process), cmp_FIFO_SJF);

	assert(assign_cpu(getpid(), P_CPU) != -1);
	assert(wake_up(getpid()) >= 0);
	
	int last_id = -1;
	int curr_time = 0;
	int rr = TQ;
	int done_count = 0;
	while(1){
	/* Wait for done process */
		if(last_id != -1 && procs[last_id].exec_time <= 0){
			waitpid(procs[last_id].pid, NULL, 0);	//kill?
			procs[last_id].pid = -1;
#ifdef DEBUG_DONE
			printf("Process %s is done before %d\n", procs[last_id].name, curr_time);
			fflush(stdout);
#endif
			done_count += 1;
			/* End of scheduling */
			if(done_count == N)
				break;
		}

	/* Fork the process who's ready */
		for(int i = 0; i < N; i++)
			if(procs[i].ready_time == curr_time){
				procs[i].pid = exec_proc(procs[i].exec_time);
				printf("%s %d\n", procs[i].name, procs[i].pid);
#ifdef DEBUG
				printf("Process %s is ready at %d\n", procs[i].name, curr_time);
				fflush(stdout);
#endif
				assert(assign_cpu(procs[i].pid, C_CPU) != -1);
				block_down(procs[i].pid);
			}
	
	/* Determine who's next */
		int curr_id = decide_proc(policy, N, procs, last_id, &rr);
#ifdef DEBUG
		printf("time = %d, curr_proc = %s\n", curr_time, procs[curr_id].name);
		fflush(stdout);
#endif
		/* Context Switch */
		if(curr_id != last_id){
			if(curr_id != -1 && procs[curr_id].pid != -1)
				wake_up(procs[curr_id].pid);
			if(last_id != -1 && procs[last_id].pid != -1)
				block_down(procs[last_id].pid);
		}

	/* Run time */
		UNIT_TIME;
		curr_time += 1;
		if(curr_id != -1){
			procs[curr_id].exec_time--;
			rr--;
		}
		last_id = curr_id;
	}
}

