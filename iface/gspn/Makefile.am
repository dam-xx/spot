AM_CPPFLAGS = -I$(top_srcdir)/src $(BUDDY_CPPFLAGS) $(LIBGSPN_CPPFLAGS)
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

lib_LTLIBRARIES = libspotgspn.la
libspotgspn_la_LIBADD = $(top_builddir)/src/libspot.la
libspotgspn_la_SOURCES = \
  gspn.hh \
  gspn.cc \
  gspnlib.h

check_PROGRAMS = dottygspn

dottygspn_SOURCES = dottygspn.cc
dottygspn_LDADD = libspotgspn.la $(LIBGSPN_LDFLAGS)
