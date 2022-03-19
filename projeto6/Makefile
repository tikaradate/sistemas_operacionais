CC = gcc
CFLAGS = -Wall -g 
 
objs = queue.o ppos_core.o
 
all: contab contab-prio

contab: $(objs)

contab-prio: $(objs)

clean:
	-rm -f $(objs) *~
 
purge: clean
	-rm -f preemp stress