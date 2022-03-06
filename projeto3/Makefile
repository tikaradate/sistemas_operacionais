CC = gcc
CFLAGS = -Wall -g
 
objs = queue.o ppos_core.o
 
all: pingpong_dispatcher
 
pingpong_dispatcher: $(objs)

clean:
	-rm -f $(objs) *~
 
purge: clean
	-rm -f pingpong_dispatcher