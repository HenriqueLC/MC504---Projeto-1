CC = gcc
CFLAGS = -g -Wall -pedantic -pthread 

PROGRAMS = baboon

all: $(PROGRAMS)

clean:
	rm -f *.o *~ $(PROGRAMS)
