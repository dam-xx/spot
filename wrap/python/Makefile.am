SUBDIRS = tests

AM_CPPFLAGS = -I$(PYTHONINC) -I$(top_srcdir)/src

python_PYTHON = $(srcdir)/spot.py
pyexec_LTLIBRARIES = _spot.la

_spot_la_SOURCES = $(srcdir)/spot_wrap.cxx
_spot_la_LDFLAGS = -avoid-version -module
_spot_la_LIBADD = $(top_builddir)/src/libspot.la

EXTRA_DIST = ltihooks.py spot.i
$(srcdir)/spot_wrap.cxx: $(srcdir)/spot.i
	swig -c++ -python -I$(top_srcdir)/src $(srcdir)/spot.i

$(srcdir)/spot.py: $(srcdir)/spot.i
	$(MAKE) $(AM_MAKEFLAGS) spot_wrap.cxx

MAINTAINERCLEANFILES = $(srcdir)/spot_wrap.cxx $(srcdir)/spot.py
