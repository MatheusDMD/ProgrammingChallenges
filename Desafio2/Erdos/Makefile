CC = g++
CFLAGS = -Wall -std=c++11
LIBS =  -lm -lrt

TARGETS = erdos

all: $(TARGETS)

erdos.o: erdos.cxx node.hxx link.hxx
	$(CC) $(CFLAGS) -c erdos.cxx

node.o: node.cxx node.hxx
	$(CC) $(CFLAGS) -c node.cxx

link.o: link.cxx link.hxx
	$(CC) $(CFLAGS) -c link.cxx

erdos: erdos.o node.o link.o
	$(CC) $(CFLAGS) -o erdos erdos.o node.o link.o

clean:
	rm *.o $(TARGETS)
