AM_CPPFLAGS = -I$(srcdir)/..

lib_LIBRARIES = libltlparse.a

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
	bison --defines --locations --skeleton=lalr1.cc --report=all \
	  $(srcdir)/$(LTLPARSE_YY) -o $@
$(FROM_LTLPARSE_YY_OTHERS): $(LTLPARSE_YY)
	@test -f $@ || $(MAKE) $(AM_MAKEFLAGS) $(FROM_LTLPARSE_YY_MAIN)

EXTRA_DIST = $(LTLPARSE_YY)

libltlparse_a_SOURCES = \
  fmterror.cc \
  $(FROM_LTLPARSE_YY) \
  ltlscan.ll \
  parsedecl.hh \
  public.hh
