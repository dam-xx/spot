AM_CPPFLAGS = -I$(srcdir)/.. $(BUDDY_CPPFLAGS)
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

miscdir = $(pkgincludedir)/misc

misc_HEADERS = \
  bddalloc.hh \
  bddlt.hh \
  hash.hh \
  minato.hh \
  version.hh

noinst_LTLIBRARIES = libmisc.la
libmisc_la_SOURCES = \
  bddalloc.cc \
  minato.cc \
  version.cc
