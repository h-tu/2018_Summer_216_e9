CC = gcc
CFLAGS = -ansi -Wall -g -O0 -Wwrite-strings -Wshadow \
	-pedantic-errors -fstack-protector-all -Wextra
LDFLAGS = -lpthread
PROGS = threads

.PHONY: all clean test

all: $(PROGS)

clean:
	rm -f *.o $(PROGS)

test:
	csh run-all-tests.csh

prof: CC += -pg
prof: $(PROGS)

threads: threads.o
	$(CC) -o threads threads.o $(LDFLAGS)

threads.o: threads.c
	$(CC) $(CFLAGS) -c threads.c
