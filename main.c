#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "scheduler.h"

int match_policy(char* policy){
	if(strcmp(policy, "FIFO") == 0)
		return FIFO;
	else if(strcmp(policy, "RR") == 0)
		return RR;
	else if(strcmp(policy, "SJF") == 0)
		return SJF;
	else if(strcmp(policy, "PSJF") == 0)
		return PSJF;
	else
		return -1;
}


int main(){
	char tmp[10];
	assert(scanf("%s", tmp) == 1);
	int policy = match_policy(tmp);
	assert(policy != -1);
	
	int N;
	assert(scanf("%d", &N) == 1);
	
	Process *procs = (Process *)malloc(N * sizeof(Process));
	for(int i = 0; i < N; i++){
		assert(scanf("%s%d%d", procs[i].name, &procs[i].ready_time, &procs[i].exec_time) == 3);
		procs[i].pid = -1;
	}

	scheduling(policy, N, procs);
}
