AM_CPPFLAGS = -I$(srcdir)/..
LDADD = ../libspot.la

check_SCRIPTS = defs
# Keep this sorted alphabetically.
check_PROGRAMS = \
  explicit \
  tgbaread

explicit_SOURCES = explicit.cc
tgbaread_SOURCES = tgbaread.cc

TESTS = \
  explicit.test \
  tgbaread.test

EXTRA_DIST = $(TESTS)

CLEANFILES = inpu stdout expected
