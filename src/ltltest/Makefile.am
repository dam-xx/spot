AM_CPPFLAGS = -I$(srcdir)/..
LDADD = ../ltlparse/libltlparse.la \
        ../ltlvisit/libltlvisit.la \
	../ltlenv/libltlenv.la \
        ../ltlast/libltlast.la

check_SCRIPTS = defs
# Keep this sorted alphabetically.
check_PROGRAMS = \
  equals \
  ltl2dot \
  ltl2text \
  lunabbrev \
  nenoform \
  tostring \
  tunabbrev \
  tunenoform

equals_SOURCES = equals.cc
ltl2dot_SOURCES = readltl.cc
ltl2dot_CPPFLAGS = $(AM_CPPFLAGS) -DDOTTY
ltl2text_SOURCES = readltl.cc
lunabbrev_SOURCES = equals.cc
lunabbrev_CPPFLAGS = $(AM_CPPFLAGS) -DLUNABBREV
nenoform_SOURCES = equals.cc
nenoform_CPPFLAGS = $(AM_CPPFLAGS) -DNENOFORM
tostring_SOURCES = tostring.cc
tunabbrev_SOURCES = equals.cc
tunabbrev_CPPFLAGS = $(AM_CPPFLAGS) -DTUNABBREV
tunenoform_SOURCES = equals.cc
tunenoform_CPPFLAGS = $(AM_CPPFLAGS) -DNENOFORM -DTUNABBREV


EXTRA_DIST = $(TESTS)

# Ordered by strength of the test.  Test basic features first.
TESTS = \
  parse.test \
  parseerr.test \
  equals.test \
  tostring.test \
  lunabbrev.test \
  tunabbrev.test \
  nenoform.test \
  tunenoform.test

CLEANFILES = stdout expect parse.dot
