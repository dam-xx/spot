DOXYGEN = doxygen

.PHONY: doc spot

all-local: $(srcdir)/stamp

doc:
	rm -f $(srcdir)/stamp
	$(MAKE) $(srcdir)/stamp

$(srcdir)/stamp: $(srcdir)/Doxyfile.in $(top_srcdir)/configure.ac
	$(MAKE) Doxyfile
	rm -rf spot.html spot.latex
	$(DOXYGEN)
	touch $@

spot.html: $(srcdir)/stamp
spot.latex: $(srcdir)/stamp

$(srcdir)/spotref.pdf: $(srcdir)/stamp
	cd $(srcdir)/spot.latex \
	  && texi2dvi --pdf refman.tex \
	  && cd .. && mv -f spot.latex/refman.pdf $@

EXTRA_DIST = $(srcdir)/stamp spot.html mainpage.dox $(srcdir)/spotref.pdf
