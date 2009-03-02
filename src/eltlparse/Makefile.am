## Copyright (C) 2008 Laboratoire d'Informatique de Paris 6 (LIP6),
## département Systèmes Répartis Coopératifs (SRC), Université Pierre
## et Marie Curie.
##
## This file is part of Spot, a model checking library.
##
## Spot is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## Spot is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
## License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Spot; see the file COPYING.  If not, write to the Free
## Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

AM_CPPFLAGS = -I$(srcdir)/..
AM_CXXFLAGS = #$(WARNING_CXXFLAGS)

eltlparsedir = $(pkgincludedir)/eltlparse

noinst_LTLIBRARIES = libeltlparse.la

ELTLPARSE_YY = eltlparse.yy
FROM_ELTLPARSE_YY_MAIN = eltlparse.cc
FROM_ELTLPARSE_YY_OTHERS = \
  stack.hh \
  position.hh \
  location.hh \
  eltlparse.hh
FROM_ELTLPARSE_YY = $(FROM_ELTLPARSE_YY_MAIN) $(FROM_ELTLPARSE_YY_OTHERS)

BUILT_SOURCES = $(FROM_ELTLPARSE_YY)
MAINTAINERCLEANFILES = $(FROM_ELTLPARSE_YY)

$(FROM_ELTLPARSE_YY_MAIN): $(srcdir)/$(ELTLPARSE_YY)
## We must cd into $(srcdir) first because if we tell bison to read
## $(srcdir)/$(ELTLPARSE_YY), it will also use the value of $(srcdir)/
## in the generated include statements.
	cd $(srcdir) && \
	  bison -Wall -Werror --report=all \
	  $(ELTLPARSE_YY) -o $(FROM_ELTLPARSE_YY_MAIN)
$(FROM_ELTLPARSE_YY_OTHERS): $(ELTLPARSE_YY)
	@test -f $@ || $(MAKE) $(AM_MAKEFLAGS) $(FROM_ELTLPARSE_YY_MAIN)

EXTRA_DIST = $(ELTLPARSE_YY)

libeltlparse_la_SOURCES = \
  fmterror.cc \
  $(FROM_ELTLPARSE_YY) \
  eltlscan.ll \
  parsedecl.hh

eltlparse_HEADERS = \
  public.hh \
  location.hh \
  position.hh
