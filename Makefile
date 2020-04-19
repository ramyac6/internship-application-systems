# Copyright 2020 Ramya Challa [legal/copyright]

CC = g++
CPPFLAGS = -Wall -g -std=c++11
PROGS = netping

all: $(PROGS)

netping: pinguino.o netping.o
	$(CC) $(CPPFLAGS) -o netping pinguino.o netping.o

pinguino.o: pinguino.cc pinguino.h
	$(CC) $(CPPFLAGS) -c pinguino.cc

netping.o: netping.cc pinguino.h
	$(CC) $(CPPFLAGS) -c netping.cc

clean:
	rm -f netping *.o *~