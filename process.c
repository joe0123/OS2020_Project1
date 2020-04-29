#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

#ifndef UNIT_TIME
#define UNIT_TIME volatile unsigned long i; for(i=0;i<1000000UL;i++)
#endif

void exec_proc(int exec_time){
        long start_time = syscall(334);
	for(int j = 0; j < exec_time; j++){
		UNIT_TIME;
	}
        long end_time = syscall(334);
	static const long BASE = 1000000000;
       	syscall(333, "[Project1] %d %ld.%.09ld %ld.%.09ld\n", getpid(), start_time / BASE, start_time % BASE, end_time / BASE, end_time % BASE);
	exit(0);
}
