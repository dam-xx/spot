AM_CPPFLAGS = -I$(srcdir)/.. $(BUDDY_CPPFLAGS)
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

tgbadir = $(pkgincludedir)/tgba

tgba_HEADERS = \
  bdddict.hh \
  bddprint.hh \
  public.hh \
  state.hh \
  statebdd.hh \
  succiter.hh \
  succiterconcrete.hh \
  tgba.hh \
  tgbabddconcrete.hh \
  tgbabddconcretefactory.hh \
  tgbabddconcreteproduct.hh \
  tgbabddcoredata.hh \
  tgbabddfactory.hh \
  tgbaexplicit.hh \
  tgbaproduct.hh

noinst_LTLIBRARIES = libtgba.la
libtgba_la_SOURCES = \
  bdddict.cc \
  bddprint.cc \
  statebdd.cc \
  succiterconcrete.cc \
  tgba.cc \
  tgbabddconcrete.cc \
  tgbabddconcretefactory.cc \
  tgbabddconcreteproduct.cc \
  tgbabddcoredata.cc \
  tgbaexplicit.cc \
  tgbaproduct.cc
