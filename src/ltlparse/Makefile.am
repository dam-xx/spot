AM_CPPFLAGS = -I$(srcdir)/..
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

ltlparsedir = $(pkgincludedir)/ltlparse

noinst_LTLIBRARIES = libltlparse.la

LTLPARSE_YY = ltlparse.yy
FROM_LTLPARSE_YY_MAIN = ltlparse.cc
FROM_LTLPARSE_YY_OTHERS = \
  stack.hh \
  position.hh \
  location.hh \
  ltlparse.hh
FROM_LTLPARSE_YY = $(FROM_LTLPARSE_YY_MAIN) $(FROM_LTLPARSE_YY_OTHERS)

BUILT_SOURCES = $(FROM_LTLPARSE_YY)
MAINTAINERCLEANFILES = $(FROM_LTLPARSE_YY)

$(FROM_LTLPARSE_YY_MAIN): $(srcdir)/$(LTLPARSE_YY)
## We must cd into $(srcdir) first because if we tell bison to read
## $(srcdir)/$(LTLPARSE_YY), it will also use the value of $(srcdir)/
## in the generated include statements.
	cd $(srcdir) && \
	  bison --defines --locations --skeleton=lalr1.cc --report=all \
	  $(LTLPARSE_YY) -o $@
$(FROM_LTLPARSE_YY_OTHERS): $(LTLPARSE_YY)
	@test -f $@ || $(MAKE) $(AM_MAKEFLAGS) $(FROM_LTLPARSE_YY_MAIN)

EXTRA_DIST = $(LTLPARSE_YY)

libltlparse_la_SOURCES = \
  fmterror.cc \
  $(FROM_LTLPARSE_YY) \
  ltlscan.ll \
  parsedecl.hh

ltlparse_HEADERS = \
  public.hh \
  location.hh \
  position.hh
