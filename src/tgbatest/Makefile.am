AM_CPPFLAGS = -I$(srcdir)/..
LDADD = ../libspot.la

check_SCRIPTS = defs
# Keep this sorted alphabetically.
check_PROGRAMS = \
  explicit \
  readsave \
  tgbaread \
  ltl2tgba \
  ltlprod \
  bddprod \
  explprod

bddprod_SOURCES = ltlprod.cc
bddprod_CXXFLAGS = -DBDD_CONCRETE_PRODUCT
explicit_SOURCES = explicit.cc
explprod_SOURCES = explprod.cc
ltl2tgba_SOURCES = ltl2tgba.cc
ltlprod_SOURCES = ltlprod.cc
readsave_SOURCES = readsave.cc
tgbaread_SOURCES = tgbaread.cc

TESTS = \
  explicit.test \
  tgbaread.test \
  readsave.test \
  ltl2tgba.test \
  ltlprod.test \
  bddprod.test \
  explprod.test

EXTRA_DIST = $(TESTS)

CLEANFILES = input input1 input2 stdout expected
