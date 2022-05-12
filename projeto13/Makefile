CC = gcc
CFLAGS = -Wall -g -lm
LDLIBS = -lrt
 
objs = queue.o ppos_core.o disk.o ppos_disk.o
EXEC = pingpong-disco2 pingpong-disco1

all: $(EXEC)

$(EXEC): $(objs)

clean:
	-rm -f $(objs) *~
 
purge: clean
	-rm -f $(EXEC)
