nodist_noinst_SCRIPTS = ltl2tgba.py
EXTRA_DIST = $(srcdir)/ltl2tgba.in README

ltl2tgba.py: $(srcdir)/ltl2tgba.in Makefile
	sed -e 's|[@]PYTHON[@]|@PYTHON@|g' \
	    -e 's|[@]pythondir[@]|@pythondir@|g' \
	    -e 's|[@]PACKAGE_VERSION[@]|@PACKAGE_VERSION@|g' \
	    <$(srcdir)/ltl2tgba.in >ltl2tgba.tmp
	chmod +x ltl2tgba.tmp
	mv -f ltl2tgba.tmp $@
