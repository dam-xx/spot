AM_CPPFLAGS = -I$(srcdir)/..
LDADD = ../libspot.la

check_SCRIPTS = defs
# Keep this sorted alphabetically.
check_PROGRAMS = \
  explicit \
  readsave \
  tgbaread \
  ltl2tgba \
  ltlprod

explicit_SOURCES = explicit.cc
ltl2tgba_SOURCES = ltl2tgba.cc
ltlprod_SOURCES = ltlprod.cc
readsave_SOURCES = readsave.cc
tgbaread_SOURCES = tgbaread.cc

TESTS = \
  explicit.test \
  tgbaread.test \
  readsave.test \
  ltl2tgba.test \
  ltlprod.test

EXTRA_DIST = $(TESTS)

CLEANFILES = input stdout expected
