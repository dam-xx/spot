AM_CPPFLAGS = -I$(srcdir)/.. $(BUDDY_CPPFLAGS)
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

tgbaalgosdir = $(pkgincludedir)/tgbaalgos

tgbaalgos_HEADERS = \
  reachiter.hh \
  dotty.hh \
  lbtt.hh \
  ltl2tgba.hh \
  save.hh

noinst_LTLIBRARIES = libtgbaalgos.la
libtgbaalgos_la_SOURCES = \
  reachiter.cc \
  dotty.cc \
  lbtt.cc \
  ltl2tgba.cc \
  save.cc
