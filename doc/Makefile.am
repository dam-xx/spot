DOXYGEN = doxygen

.PHONY: doc spot

all-local: stamp

doc:
	rm stamp
	$(AMKE) stamp

stamp: Doxyfile.in ../configure.ac
	$(MAKE) Doxyfile
	rm -rf spot.html spot.latex
	$(DOXYGEN)
	: > $@

spot.html: stamp
spot.latex: stamp

EXTRA_DIST = stamp spot.html
