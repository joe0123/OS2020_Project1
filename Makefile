all: scheduler calc
scheduler:
	gcc -O2 main.c scheduler.c process.c -o scheduler
calc:
	gcc calc.c -o calc
clean:
	rm scheduler calc
