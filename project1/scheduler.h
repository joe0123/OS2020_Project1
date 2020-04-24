#define FIFO	0
#define RR	1
#define SJF	2
#define PSJF	3

#define	P_CPU	0
#define C_CPU	1

#define MIN_READY 0
#define MIN_EXEC 1

#ifndef UNIT_TIME
#define UNIT_TIME volatile unsigned long i; for(i=0;i<1000000UL;i++)
#endif

#define TQ 500

int assign_cpu(int t, int core);
int scheduling(int policy, int N, Process *processes);
