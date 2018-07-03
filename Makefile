CC = gcc 
CFLAGS = -O2 -Wall -W -std=gnu99 -Wshadow
TARGETS = server
 

all: $(TARGETS)


server: server.o sockwrap.o engine.o
targets: server




clean:
	rm -f *.o

distclean: clean
	rm -f $(TARGETS)
