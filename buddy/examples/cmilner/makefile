# ---------------------------------
# Makefile for cmilner test example
# ---------------------------------

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

cmilner: cmilner.o bddlib
	$(CC) $(CFLAGS) cmilner.o -o cmilner -lbdd -lm

bddlib:
	cd ../../src ; make

clean:
	rm -f *~
	rm -f *.o
	rm -f cmilner

cmilner.o:	../../src/bdd.h
