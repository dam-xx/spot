DOXYGEN = doxygen

.PHONY: doc spot

all-local: $(srcdir)/stamp

doc:
	rm $(srcdir)/stamp
	$(AMKE) $(srcdir)/stamp

$(srcdir)/stamp: $(srcdir)/Doxyfile.in $(top_srcdir)/configure.ac
	$(MAKE) Doxyfile
	rm -rf spot.html spot.latex
	$(DOXYGEN)
	touch $@

spot.html: $(srcdir)/stamp
spot.latex: $(srcdir)/stamp

EXTRA_DIST = $(srcdir)/stamp spot.html
