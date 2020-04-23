#include <unistd.h>

typedef struct Process{
	char name[32];
	int ready_time;
	int exec_time;
	pid_t pid;
}Process;

int exec_process();

