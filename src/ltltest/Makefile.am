AM_CPPFLAGS = -I$(srcdir)/..
LDADD = ../ltlparse/libltlparse.a \
        ../ltlvisit/libltlvisit.a \
        ../ltlast/libltlast.a 

check_SCRIPTS = defs
check_PROGRAMS = ltl2dot ltl2text equals lunabbrev tunabbrev
ltl2dot_SOURCES = readltl.cc
ltl2dot_CPPFLAGS = $(AM_CPPFLAGS) -DDOTTY
ltl2text_SOURCES = readltl.cc
equals_SOURCES = equals.cc
lunabbrev_SOURCES = equals.cc
lunabbrev_CPPFLAGS = $(AM_CPPFLAGS) -DLUNABBREV
tunabbrev_SOURCES = equals.cc
tunabbrev_CPPFLAGS = $(AM_CPPFLAGS) -DTUNABBREV

EXTRA_DIST = $(TESTS)
TESTS = parse.test parseerr.test equals.test lunabbrev.test tunabbrev.test

CLEANFILES = stdout expect parse.dot
