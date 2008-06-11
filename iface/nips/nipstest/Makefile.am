## Copyright (C) 2003, 2004, 2005, 2006, 2008 Laboratoire
## d'Informatique de Paris 6 (LIP6), département Systèmes Répartis
## Coopératifs (SRC), Université Pierre et Marie Curie.
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


check_SCRIPTS = defs

BYTECODES = dinner.pr.nips.b never.pr.b peterson.pm.b regbit.b
EXTRA_DIST = $(TESTS) $(BYTECODES)

# Ordered by strength of the test.  Test basic features first.
TESTS = \
dotty.test \
emptiness.test

CLEANFILES = \
  dotty
