AM_CPPFLAGS = -I$(top_srcdir)/src $(BUDDY_CPPFLAGS) $(LIBGSPN_CPPFLAGS)
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

gspndir = $(pkgincludedir)/gspn

gspn_HEADERS = \
  gspn.hh

lib_LTLIBRARIES = libspotgspn.la
libspotgspn_la_LIBADD = $(top_builddir)/src/libspot.la
libspotgspn_la_SOURCES = \
  gspn.cc

check_PROGRAMS = \
  dottygspn-rg \
  dottygspn-srg \
  ltlgspn-rg \
  ltlgspn-srg \
  eltlgspn-srg \
  fmgspn-rg \
  fmgspn-srg

dottygspn_rg_SOURCES = dottygspn.cc
dottygspn_rg_LDADD = libspotgspn.la $(LIBGSPNRG_LDFLAGS)

dottygspn_srg_SOURCES = dottygspn.cc
dottygspn_srg_LDADD = libspotgspn.la $(LIBGSPNSRG_LDFLAGS)

ltlgspn_rg_SOURCES = ltlgspn.cc
ltlgspn_rg_LDADD = libspotgspn.la $(LIBGSPNRG_LDFLAGS)

ltlgspn_srg_SOURCES = ltlgspn.cc
ltlgspn_srg_LDADD = libspotgspn.la $(LIBGSPNSRG_LDFLAGS)

eltlgspn_srg_SOURCES = ltlgspn.cc
eltlgspn_srg_LDADD = libspotgspn.la $(LIBGSPNSRG_LDFLAGS)
eltlgspn_srg_CPPFLAGS = -DCEC $(AM_CPPFLAGS)

fmgspn_rg_SOURCES = ltlgspn.cc
fmgspn_rg_CPPFLAGS = -DFM $(AM_CPPFLAGS)
fmgspn_rg_LDADD = libspotgspn.la $(LIBGSPNRG_LDFLAGS)

fmgspn_srg_SOURCES = ltlgspn.cc
fmgspn_srg_CPPFLAGS = -DFM $(AM_CPPFLAGS)
fmgspn_srg_LDADD = libspotgspn.la $(LIBGSPNSRG_LDFLAGS)

EXTRA_DIST = \
  examples/DCSwave/DCSWave.def \
  examples/DCSwave/DCSWave.net \
  examples/DCSwave/DCSWave.tobs \
  examples/simple/simple.def \
  examples/simple/simple.net \
  examples/simple/simple.tobs \
  examples/udcs/udcs.def \
  examples/udcs/udcs.net \
  examples/udcs/udcs.tobs \
  $(TESTS)

TESTS = \
  simple.test \
  dcswave.test \
  dcswaveltl.test \
  dcswavefm.test \
  dcswaveeltl.test \
  udcsltl.test \
  udcseltl.test
XFAIL_TESTS = udcseltl.test

# Each test case depends on defs.
check_SCRIPTS = defs

distclean-local:
	-rm -rf testSubDir
