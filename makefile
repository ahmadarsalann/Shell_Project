C = gcc
CFLAGS = -g -Wall

default: my_shell

my_shell: my_shell.o main.o
	$(CC) $(CFLAGS) -o my_shell my_shell.o main.o

my_shell.o: my_shell.c my_shell.h
	$(CC) $(CFLAGS) -c my_shell.c

clean:
	rm -f procprog *.o
