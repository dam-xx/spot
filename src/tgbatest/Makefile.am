AM_CPPFLAGS = -I$(srcdir)/..
LDADD = ../libspot.la

check_SCRIPTS = defs
# Keep this sorted alphabetically.
check_PROGRAMS = \
  explicit \
  readsave \
  tgbaread


explicit_SOURCES = explicit.cc
readsave_SOURCES = readsave.cc
tgbaread_SOURCES = tgbaread.cc

TESTS = \
  explicit.test \
  tgbaread.test \
  readsave.test

EXTRA_DIST = $(TESTS)

CLEANFILES = inpu stdout expected
