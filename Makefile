CC = gcc
CFLAGS = -Wall -pedantic -std=gnu99

.PHONY: all clean

all: hostd

hostd: hostd.c pcb.c mab.c
	$(CC) $(CFLAGS) -g $^ -o $@

clean:
	-rm -f *.o
	-rm -f host
