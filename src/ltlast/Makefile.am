AM_CPPFLAGS = -I$(srcdir)/..
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

ltlastdir = $(pkgincludedir)/ltlast

ltlast_HEADERS = \
  allnodes.hh \
  atomic_prop.hh \
  binop.hh \
  constant.hh \
  formula.hh \
  multop.hh \
  predecl.hh \
  refformula.hh \
  unop.hh \
  visitor.hh

noinst_LTLIBRARIES = libltlast.la
libltlast_la_SOURCES = \
  atomic_prop.cc \
  binop.cc \
  constant.cc \
  formula.cc \
  multop.cc \
  refformula.cc \
  unop.cc

