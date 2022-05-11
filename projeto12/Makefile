CC = gcc
CFLAGS = -Wall -g -lm
LDLIBS = -lm
 
objs = queue.o ppos_core.o mqueue.o
EXEC = mqueue

all: $(EXEC)

$(EXEC): $(objs)

clean:
	-rm -f $(objs) *~
 
purge: clean
	-rm -f $(EXEC)
