AM_CPPFLAGS = -I$(srcdir)/..
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

noinst_LTLIBRARIES = libtgbaparse.la

TGBAPARSE_YY = tgbaparse.yy
FROM_TGBAPARSE_YY_MAIN = tgbaparse.cc
FROM_TGBAPARSE_YY_OTHERS = \
  stack.hh \
  position.hh \
  location.hh \
  tgbaparse.hh
FROM_TGBAPARSE_YY = $(FROM_TGBAPARSE_YY_MAIN) $(FROM_TGBAPARSE_YY_OTHERS)

BUILT_SOURCES = $(FROM_TGBAPARSE_YY)
MAINTAINERCLEANFILES = $(FROM_TGBAPARSE_YY)

$(FROM_TGBAPARSE_YY_MAIN): $(srcdir)/$(TGBAPARSE_YY)
	bison --defines --locations --skeleton=lalr1.cc --report=all \
	  $(srcdir)/$(TGBAPARSE_YY) -o $@
$(FROM_TGBAPARSE_YY_OTHERS): $(TGBAPARSE_YY)
	@test -f $@ || $(MAKE) $(AM_MAKEFLAGS) $(FROM_TGBAPARSE_YY_MAIN)

EXTRA_DIST = $(TGBAPARSE_YY)

libtgbaparse_la_SOURCES = \
  fmterror.cc \
  $(FROM_TGBAPARSE_YY) \
  tgbascan.ll \
  parsedecl.hh \
  public.hh
