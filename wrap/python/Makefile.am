SUBDIRS = . cgi tests

AM_CPPFLAGS = -I$(PYTHONINC) -I$(top_srcdir)/src $(BUDDY_CPPFLAGS)

EXTRA_DIST = ltihooks.py spot.i buddy.i
python_PYTHON = $(srcdir)/spot.py $(srcdir)/buddy.py
pyexec_LTLIBRARIES = _spot.la _buddy.la
lib_LTLIBRARIES = libspotswigpy.la

libspotswigpy_la_SOURCES = libpy.c
libspotswigpy_la_CFLAGS = -DSWIG_GLOBAL
libspotswigpy_la_LDFLAGS = -avoid-version

_spot_la_SOURCES = $(srcdir)/spot_wrap.cxx
_spot_la_LDFLAGS = -avoid-version -module
_spot_la_LIBADD = $(top_builddir)/src/libspot.la libspotswigpy.la

$(srcdir)/spot_wrap.cxx: $(srcdir)/spot.i
	swig -c -c++ -python -I$(srcdir) -I$(top_srcdir)/src $(srcdir)/spot.i

$(srcdir)/spot.py: $(srcdir)/spot.i
	$(MAKE) $(AM_MAKEFLAGS) spot_wrap.cxx

_buddy_la_SOURCES = $(srcdir)/buddy_wrap.cxx
_buddy_la_LDFLAGS = -avoid-version -module $(BUDDY_LDFLAGS) libspotswigpy.la

$(srcdir)/buddy_wrap.cxx: $(srcdir)/buddy.i
	swig -c -c++ -python $(BUDDY_CPPFLAGS) $(srcdir)/buddy.i

$(srcdir)/buddy.py: $(srcdir)/buddy.i
	$(MAKE) $(AM_MAKEFLAGS) buddy_wrap.cxx


MAINTAINERCLEANFILES = $(srcdir)/spot_wrap.cxx $(srcdir)/spot.py
