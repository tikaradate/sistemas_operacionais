CC = gcc
CFLAGS = -Wall -g
 
objs = queue.o ppos_core.o
 
all: sched
 
sched: $(objs)

clean:
	-rm -f $(objs) *~
 
purge: clean
	-rm -f sched