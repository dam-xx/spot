AM_CPPFLAGS = -I$(srcdir)/..
LDADD = ../libspot.la

check_SCRIPTS = defs
# Keep this sorted alphabetically.
check_PROGRAMS = \
  explicit

explicit_SOURCES = explicit.cc

TESTS = \
  explicit.test

EXTRA_DIST = $(TESTS)

CLEANFILES = stdout expected
