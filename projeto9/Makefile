CC = gcc
CFLAGS = -Wall -g 
 
objs = queue.o ppos_core.o
 
all: sleep

sleep: $(objs)

clean:
	-rm -f $(objs) *~
 
purge: clean
	-rm -f sleep
