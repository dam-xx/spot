AM_CPPFLAGS = -I$(srcdir)/.. $(BUDDY_CPPFLAGS)
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

tgbadir = $(pkgincludedir)/tgba

tgba_HEADERS = \
  bddfactory.hh \
  bddprint.hh \
  dictunion.hh \
  ltl2tgba.hh \
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
  tgbabdddict.hh \
  tgbabddfactory.hh \
  tgbabddtranslatefactory.hh \
  tgbaexplicit.hh \
  tgbaproduct.hh \
  tgbatranslateproxy.hh

noinst_LTLIBRARIES = libtgba.la
libtgba_la_SOURCES = \
  bddfactory.cc \
  bddprint.cc \
  dictunion.cc \
  ltl2tgba.cc \
  statebdd.cc \
  succiterconcrete.cc \
  tgbabddconcrete.cc \
  tgbabddconcretefactory.cc \
  tgbabddconcreteproduct.cc \
  tgbabddcoredata.cc \
  tgbabdddict.cc \
  tgbabddtranslatefactory.cc \
  tgbaexplicit.cc \
  tgbaproduct.cc \
  tgbatranslateproxy.cc
