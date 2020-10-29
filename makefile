CC=gcc
CFLAGS=-Wall -std=gnu99

all: smallsh

smallsh: smallsh.o
	gcc -g $(CFLAGS) -o smallsh smallsh.o

smallsh.o: smallsh.c
	gcc -g $(CFLAGS) -c smallsh.c

clean:
	-rm *.o

cleanall: clean
	-rm smallsh