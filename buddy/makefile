# ==============================================================
# Makefile for the BuDDy package
# - Do not touch. Edit "config" instead.
# ==============================================================

include config

TARGET = buddy
RD = $(TARGET)$(VERSION)


# --------------------------------------------------------------
# The primary targets.
# --------------------------------------------------------------

all: buddy examples

buddy:
	cd src; make

docs:
	cd doc; make doc

install:
	cp -f src/libbdd.a $(LIBDIR)/libbdd.a
	chmod 644 $(LIBDIR)/libbdd.a
	cp -f src/bdd.h $(INCDIR)/bdd.h
	chmod 644 $(INCDIR)/bdd.h
	cp -f src/fdd.h $(INCDIR)/fdd.h
	chmod 644 $(INCDIR)/fdd.h
	cp -f src/bvec.h $(INCDIR)/bvec.h
	chmod 644 $(INCDIR)/bvec.h

uninstall:
	rm -f $(LIBDIR)/libbdd.a
	rm -f $(INCDIR)/bdd.h
	rm -f $(INCDIR)/fdd.h
	rm -f $(INCDIR)/bvec.h


# --------------------------------------------------------------
# Housekeeping
# --------------------------------------------------------------

clean:
	cd examples/milner; make clean
	cd examples/cmilner; make clean
	cd examples/queen; make clean
	cd examples/adder; make clean
	cd examples/fdd; make clean
	cd examples/bddcalc; make clean
	cd examples/solitare; make clean
	cd examples/money; make clean
	cd examples/bddtest; make clean
	cd src; make clean
	rm -f *~

examples: dummy
	cd examples/milner; make
	cd examples/cmilner; make
	cd examples/queen; make
	cd examples/adder; make
	cd examples/fdd; make
	cd examples/bddcalc; make
	cd examples/solitare; make
	cd examples/money; make
	cd examples/bddtest; make

dummy:


