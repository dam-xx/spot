## Copyright (C) 2008 Laboratoire de Recherche et Développement
## de l'Epita (LRDE).
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

TEMPLATES =
MODELS =

include modelgen.mk
include bench.mk

EXTRA_DIST = 					\
  $(TEMPLATES)					\
  templates/common				\
  templates/gen					\
  README

run_bench = tools/runbench

dist_noinst_SCRIPTS = \
  tools/bench-create \
  tools/trans2prop.pl \
  tools/sum \
  $(run_bench)

$(RESULTS): defs
nodist_check_DATA = $(RESULTS)

clean-local:
	rm -rf models results
