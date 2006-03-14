## Copyright (C) 2006  Laboratoire d'Informatique de Paris 6 (LIP6),
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

AM_CPPFLAGS = -I$(srcdir)/.. $(BUDDY_CPPFLAGS)
AM_CXXFLAGS = $(WARNING_CXXFLAGS)

sautparsedir = $(pkgincludedir)/sautparse

sautparse_HEADERS = \
  public.hh

noinst_LTLIBRARIES = libsautparse.la

SAUTPARSE_YY = sautparse.yy
FROM_SAUTPARSE_YY_MAIN = sautparse.cc
FROM_SAUTPARSE_YY_OTHERS = \
  stack.hh \
  position.hh \
  location.hh \
  sautparse.hh

FROM_SAUTPARSE_YY = $(FROM_SAUTPARSE_YY_MAIN) $(FROM_SAUTPARSE_YY_OTHERS)

BUILT_SOURCES = $(FROM_SAUTPARSE_YY)
MAINTAINERCLEANFILES = $(FROM_SAUTPARSE_YY)

$(FROM_SAUTPARSE_YY_MAIN): $(srcdir)/$(SAUTPARSE_YY)
## We must cd into $(srcdir) first because if we tell bison to read
## $(srcdir)/$(SAUTPARSE_YY), it will also use the value of $(srcdir)/
## in the generated include statements.
	cd $(srcdir) && \
	  bison --defines --locations --skeleton=lalr1.cc --report=all \
	  $(SAUTPARSE_YY) -o $(FROM_SAUTPARSE_YY_MAIN)
$(FROM_SAUTPARSE_YY_OTHERS): $(SAUTPARSE_YY)
	@test -f $@ || $(MAKE) $(AM_MAKEFLAGS) $(FROM_SAUTPARSE_YY_MAIN)

EXTRA_DIST = $(SAUTPARSE_YY)

libsautparse_la_SOURCES = \
  fmterror.cc \
  $(FROM_SAUTPARSE_YY) \
  sautscan.ll \
  parsedecl.hh \
  result.cc
