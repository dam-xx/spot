# --------------------------------
# Makefile for FDD test example
# --------------------------------

# --- Compiler flags
CFLAGS = -O3 -pedantic -Wall -ansi -L../../src -I../../src

# --- C++ compiler
CPP = g++

# --- C compiler
CC = gcc


# --- Do not touch ---

.SUFFIXES: .cxx .c

.cxx.o:
	$(CPP) $(CFLAGS) -c $<

.c.o:
	$(CC) $(CFLAGS) -c $<

fdd:	fdd.o bddlib
	$(CPP) $(CFLAGS) fdd.o -o fdd -lbdd -lm

bddlib:
	cd ../../src; make

clean:
	rm -f *~
	rm -f *.o
	rm -f fdd

fdd.o:	../../src/fdd.h
