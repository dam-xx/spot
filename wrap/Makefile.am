
AM_CPPFLAGS = -I$(PYTHONINC) -I$(top_srcdir)/src

python_PYTHON = spot.py
pyexec_LTLIBRARIES = _spot.la

_spot_la_SOURCES = spot_wrap.cxx
_spot_la_LDFLAGS = -avoid-version -module 
_spot_la_LIBADD = \
  ../src/ltlenv/libltlenv.la \
  ../src/ltlparse/libltlparse.la \
  ../src/ltlvisit/libltlvisit.la \
  ../src/ltlast/libltlast.la

EXTRA_DIST = spot.i
spot_wrap.cxx: spot.i
	swig -c++ -python -I$(top_srcdir)/src spot.i

spot.py: spot.i
	$(MAKE) $(AM_MAKEFLAGS) spot_wrap.cxx

MAINTAINERCLEANFILES = spot_wrap.cxx spot.py
