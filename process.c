#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/syscall.h>

#ifndef UNIT_TIME
#define UNIT_TIME volatile unsigned long i; for(i=0;i<1000000UL;i++)
#endif

int main(int argc, char* argv[]){
	int exec_time = atoi(argv[0]);
	struct timespec start, end;
        syscall(228, CLOCK_REALTIME, &start);
	while(exec_time > 0){
		UNIT_TIME;
		exec_time--;
	}
        syscall(228, CLOCK_REALTIME, &end);
       	syscall(333, "[Project1] %d %ld.%.09ld %ld.%.09ld\n", getpid(), start.tv_sec, start.tv_nsec, end.tv_sec, end.tv_nsec);
	exit(0);
}

