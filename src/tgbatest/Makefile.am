AM_CPPFLAGS = -I$(srcdir)/.. $(BUDDY_CPPFLAGS)
LDADD = ../libspot.la

check_SCRIPTS = defs
# Keep this sorted alphabetically.
check_PROGRAMS = \
  bddprod \
  explicit \
  explprod \
  ltl2tgba \
  ltlmagic \
  ltlprod \
  mixprod \
  readsave \
  spotlbtt \
  tbalbtt \
  tgbaread \
  tripprod

# Keep this sorted alphabetically.
bddprod_SOURCES  = ltlprod.cc
bddprod_CXXFLAGS = -DBDD_CONCRETE_PRODUCT
explicit_SOURCES = explicit.cc
explprod_SOURCES = explprod.cc
ltl2tgba_SOURCES = ltl2tgba.cc
ltlmagic_SOURCES = ltlmagic.cc
ltlprod_SOURCES  = ltlprod.cc
mixprod_SOURCES  = mixprod.cc
readsave_SOURCES = readsave.cc
tbalbtt_SOURCES  = spotlbtt.cc
tbalbtt_CXXFLAGS = -DTBA
tgbaread_SOURCES = tgbaread.cc
tripprod_SOURCES = tripprod.cc
spotlbtt_SOURCES = spotlbtt.cc

# Keep this sorted by STRENGTH.  Test basic things first,
# because such failures will be easier to diagnose and fix.
TESTS = \
  explicit.test \
  tgbaread.test \
  readsave.test \
  ltl2tgba.test \
  ltlprod.test \
  bddprod.test \
  explprod.test \
  tripprod.test \
  mixprod.test \
  spotlbtt.test \
  ltlmagic.test

EXTRA_DIST = $(TESTS)

CLEANFILES = input input1 input2 input3 stdout expected config
