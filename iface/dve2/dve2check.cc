// Copyright (C) 2011 Laboratoire de Recherche et Developpement de
// l'Epita (LRDE)
//
// This file is part of Spot, a model checking library.
//
// Spot is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Spot is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Spot; see the file COPYING.  If not, write to the Free
// Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#include "dve2.hh"
#include "tgbaalgos/dotty.hh"
#include "ltlenv/defaultenv.hh"
#include "ltlast/allnodes.hh"

int
main(int argc, char **argv)
{
  spot::ltl::default_environment& env =
    spot::ltl::default_environment::instance();

  spot::ltl::atomic_prop_set ap;

  if (argc <= 1)
    {
      std::cerr << "usage: " << argv[0] << " model" << std::endl;
      exit(1);
    }

  while (argc > 2)
    {
      ap.insert(static_cast<spot::ltl::atomic_prop*>
		(env.require(argv[argc - 1])));
      --argc;
    }

  spot::bdd_dict* dict = new spot::bdd_dict();
  spot::kripke* a = spot::load_dve2(argv[1], dict, &ap, true);

  for (spot::ltl::atomic_prop_set::const_iterator it = ap.begin();
       it != ap.end(); ++it)
    (*it)->destroy();
  ap.clear();

  if (!a)
    {
      delete dict;
      exit(1);
    }

  spot::dotty_reachable(std::cout, a);

  delete a;

  spot::ltl::atomic_prop::dump_instances(std::cerr);
  spot::ltl::unop::dump_instances(std::cerr);
  spot::ltl::binop::dump_instances(std::cerr);
  spot::ltl::multop::dump_instances(std::cerr);
  spot::ltl::automatop::dump_instances(std::cerr);
  assert(spot::ltl::atomic_prop::instance_count() == 0);
  assert(spot::ltl::unop::instance_count() == 0);
  assert(spot::ltl::binop::instance_count() == 0);
  assert(spot::ltl::multop::instance_count() == 0);
  assert(spot::ltl::automatop::instance_count() == 0);

  delete dict;
}
