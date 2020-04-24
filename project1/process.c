#include <stdlib.h>
#include <unistd.h>

#ifndef UNIT_TIME
#define UNIT_TIME volatile unsigned long i; for(i=0;i<1000000UL;i++)
#endif


int exec_proc(int exec_time){
	pid_t pid = fork();
	if(pid == 0){
		while(exec_time > 0){
			UNIT_TIME;
			exec_time--;
		}
		exit(0);
	}
	else
		return pid;
}

