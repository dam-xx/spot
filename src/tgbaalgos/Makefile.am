AM_CPPFLAGS = -I$(srcdir)/.. $(BUDDY_CPPFLAGS)
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

tgbaalgosdir = $(pkgincludedir)/tgbaalgos

tgbaalgos_HEADERS = \
  dotty.hh \
  dupexp.hh \
  emptinesscheck.hh \
  lbtt.hh \
  ltl2tgba_fm.hh \
  ltl2tgba_lacim.hh \
  magic.hh \
  reachiter.hh \
  save.hh

noinst_LTLIBRARIES = libtgbaalgos.la
libtgbaalgos_la_SOURCES = \
  dotty.cc \
  dupexp.cc \
  emptinesscheck.cc \
  lbtt.cc \
  ltl2tgba_fm.cc \
  ltl2tgba_lacim.cc \
  magic.cc \
  reachiter.cc \
  save.cc
