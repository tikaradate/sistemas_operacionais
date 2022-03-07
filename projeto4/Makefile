CC = gcc
CFLAGS = -Wall -g
 
objs = queue.o ppos_core.o
 
all: pingpong_scheduler
 
pingpong_scheduler: $(objs)

clean:
	-rm -f $(objs) *~
 
purge: clean
	-rm -f pingpong_scheduler