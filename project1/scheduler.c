#include <unistd.h>
#include <sched.h>
#include <assert.h>
#include "scheduler.h"


inline int assign_cpu(int pid, int core){
	if (core > sizeof(cpu_set_t))
		return -1;

	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(core, &mask);

	if (sched_setaffinity(pid, sizeof(mask), &mask) < 0)
		return -1;

	return 0;
}

inline int wake_up(int pid){
    struct sched_param param;
    param.sched_priority = 0;
    return sched_setscheduler(pid, SCHED_OTHER, &param);	/* SCHED_OTHER: Standard Round-Robin*/
}

inline int block_down(int pid){
    struct sched_param param;
    param.sched_priority = 0;
    return sched_setscheduler(pid, SCHED_IDLE, &param);	/* SCHED_IDLE: with very low priority*/
}


inline int decide_proc(int policy, int N, Process* procs, int last_id, int rr){
	int curr_id = last_id;
	if(curr_id != -1 && procs[curr_id].pid == -1)
		curr_id = -1;

	switch(policy){
		case FIFO:
			if(curr_id == -1)
				for(int i = 0; i < N; i++)
					if(curr_id == -1 || (procs[i].pid != -1 && (procs[i].ready_time < procs[curr_id].ready_time || (procs[i].ready_time == procs[curr_id].ready_time && procs[i].exec_time < procs[curr_id].exec_time))))
						curr_id = i;
			break;
		case RR:
			if(curr_id == -1 || rr == 0)
				for(int i = 0; i < N; i++)
					if(curr_id == -1 || (procs[i].pid != -1 && (procs[i].ready_time < procs[curr_id].ready_time || (procs[i].ready_time == procs[curr_id].ready_time && procs[i].exec_time < procs[curr_id].exec_time))))
						curr_id = i;
			break;
		case SJF:
			if(curr_id == -1)
				for(int i = 0; i < N; i++)
					if(curr_id == -1 || (procs[i].pid != -1 && (procs[i].exec_time < procs[curr_id].exec_time || (procs[i].exec_time == procs[curr_id].exec_time && procs[i].ready_time < procs[curr_id].ready_time))))
						curr_id = i;
			break;
		case PSJF:
			for(int i = 0; i < N; i++)
				if(curr_id == -1 || (procs[i].pid != -1 && (procs[i].exec_time < procs[curr_id].exec_time || (procs[i].exec_time == procs[curr_id].exec_time && procs[i].ready_time < procs[curr_id].ready_time))))
					curr_id = i;
			break;
	}

	return curr_id;
}



int scheduling(int policy, int N, Process *procs){
	assert(assign_cpu(getpid(), P_CPU) != -1);
	assert(wake_up(getpid()) >= 0);
	
	int last_id = -1
	int curr_time = 0
	int done_count = 0
	while(1){
	/* Wait for done process */
		if(last_pid != -1 && procs[last_id].exec_time <= 0){
			waitpid(procs[last_pid].pid, NULL, 0);	//kill?
			procs[last_pid].pid = -1;
			done_count += 1;
			/* End of scheduling */
			if(done_count == N)
				break;
		}

	/* Fork the process who's ready */
		for(int i = 0; i < N; i++)
			if(procs[i].ready_time == curr_time){
				procs[i].pid = exec_proc(procs[i].exec_time);
				assert(assign_cpu(procs[i].pid, C_CPU) != -1);
				block_down(procs[i].pid);
			}
	
	/* Determine who's next */
		int curr_id = decide_proc();
		/* Context Switch */
		if(curr_id != last_id){
			if(curr_id != -1 && procs[curr_id].pid != -1)
				wake_up(procs[curr_id].pid);
			if(last_id != -1 && procs[last_id].pid != -1)
				block_down(procs[last_id].pid);
			rr = 0;
		}

	/* Run time */
		UNIT_TIME;
		curr_time += 1;
		if(curr_id != -1){
			procs[curr_id].exec_time--;
			rr = (rr + 1) % TQ;
		}
		last_id = curr_id;
	}
}

