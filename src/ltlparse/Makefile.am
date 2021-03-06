## Copyright (C) 2008, 2009, 2010, 2011 Laboratoire de Recherche et
## Développement de l'Epita (LRDE).
## Copyright (C) 2003, 2004 Laboratoire d'Informatique de Paris
## 6 (LIP6), département Systèmes Répartis Coopératifs (SRC),
## Université Pierre et Marie Curie.
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

AM_CPPFLAGS = -I$(srcdir)/.. -DYY_NO_INPUT
# Disable -Werror because too many versions of flex yield warnings.
AM_CXXFLAGS = $(WARNING_CXXFLAGS:-Werror=)

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
	  bison -Wall -Werror --report=all \
	  $(LTLPARSE_YY) -o $(FROM_LTLPARSE_YY_MAIN)
$(FROM_LTLPARSE_YY_OTHERS): $(LTLPARSE_YY)
	@test -f $@ || $(MAKE) $(AM_MAKEFLAGS) $(FROM_LTLPARSE_YY_MAIN)

EXTRA_DIST = $(LTLPARSE_YY)

libltlparse_la_SOURCES = \
  fmterror.cc \
  $(FROM_LTLPARSE_YY) \
  ltlfile.cc \
  ltlscan.ll \
  parsedecl.hh

ltlparse_HEADERS = \
  ltlfile.hh \
  location.hh \
  public.hh \
  position.hh
