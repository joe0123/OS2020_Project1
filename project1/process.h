#include <unistd.h>

typedef struct Process{
	char name[32];
	int ready_time;
	int exec_time;
	pid_t pid;
	int ready;
}Process;

int exec_proc(int exec_time);

