all: scheduler process calc
scheduler:
	gcc  main.c scheduler.c -o scheduler
process:
	gcc  process.c -o process
calc:
	gcc calc.c -o calc
clean:
	rm scheduler process calc
