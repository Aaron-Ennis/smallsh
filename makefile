CC=gcc
CFLAGS=-Wall -std=gnu99

all: smallsh

smallsh: smallsh.o linkedList.o command.o
	gcc -g $(CFLAGS) -o smallsh smallsh.o linkedList.o command.o

linkedList.o: linkedList.c linkedList.h
	gcc -g ${CFLAGS} -c linkedList.c

command.o: command.c command.h
	gcc -g ${CFLAGS} -c command.c

smallsh.o: smallsh.c linkedList.h
	gcc -g $(CFLAGS) -c smallsh.c

clean:
	-rm *.o

cleanall: clean
	-rm smallsh