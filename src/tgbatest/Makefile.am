AM_CPPFLAGS = -I$(srcdir)/..
LDADD = ../libspot.la

check_SCRIPTS = defs
# Keep this sorted alphabetically.
check_PROGRAMS = \
  explicit \
  readsave \
  tgbaread \
  ltl2tgba

explicit_SOURCES = explicit.cc
ltl2tgba_SOURCES = ltl2tgba.cc
readsave_SOURCES = readsave.cc
tgbaread_SOURCES = tgbaread.cc

TESTS = \
  explicit.test \
  tgbaread.test \
  readsave.test \
  ltl2tgba.test

EXTRA_DIST = $(TESTS)

CLEANFILES = inpu stdout expected
