AM_CPPFLAGS = -I$(srcdir)/.. $(BUDDY_CPPFLAGS)
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

tgbaalgosdir = $(pkgincludedir)/tgbaalgos

tgbaalgos_HEADERS = \
  reachiter.hh \
  dotty.hh \
  lbtt.hh \
  ltl2tgba_fm.hh \
  ltl2tgba_lacim.hh \
  magic.hh \
  emptinesscheck.hh \
  save.hh

noinst_LTLIBRARIES = libtgbaalgos.la
libtgbaalgos_la_SOURCES = \
  reachiter.cc \
  dotty.cc \
  lbtt.cc \
  ltl2tgba_fm.cc \
  ltl2tgba_lacim.cc \
  magic.cc \
  emptinesscheck.cc \
  save.cc
