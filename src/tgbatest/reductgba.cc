// Copyright (C) 2003, 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
// département Systèmes Répartis Coopératifs (SRC), Université Pierre
// et Marie Curie.
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

#include "tgbaalgos/ltl2tgba_fm.hh"
#include "tgbaalgos/reductgba_sim.hh"
#include "tgba/tgbareduc.hh"

#include "ltlvisit/destroy.hh"
#include "ltlvisit/reducform.hh"
#include "ltlast/allnodes.hh"
#include "ltlparse/public.hh"
#include "tgbaalgos/ltl2tgba_lacim.hh"
#include "tgbaalgos/ltl2tgba_fm.hh"
#include "tgba/bddprint.hh"
#include "tgbaalgos/dotty.hh"
#include "tgbaalgos/lbtt.hh"
#include "tgba/tgbatba.hh"
#include "tgbaalgos/magic.hh"
#include "tgbaalgos/gtec/gtec.hh"
#include "tgbaalgos/gtec/ce.hh"
#include "tgbaparse/public.hh"
#include "tgbaalgos/dupexp.hh"
#include "tgbaalgos/neverclaim.hh"

#include "misc/escape.hh"

void
syntax(char* prog)
{
  std::cerr << prog << " option formula1" << std::endl;
  exit(2);
}

int
main(int argc, char** argv)
{
  if (argc < 2)
    syntax(argv[0]);

  int exit_code = 0;
  spot::simulation_relation* rel = NULL;
  spot::tgba* automata = NULL;
  spot::tgba* aut_red = NULL;
  spot::tgba_reduc* automatareduc = NULL;

  spot::ltl::environment& env(spot::ltl::default_environment::instance());
  spot::bdd_dict* dict = new spot::bdd_dict();
  spot::ltl::parse_error_list p1;
  spot::ltl::formula* f = spot::ltl::parse(argv[1], p1, env);

  //std::cout << "Compute the automata" << std::endl;
  automata = spot::ltl_to_tgba_fm(f, dict,
				  false, true,
				  false, true);

  //std::cout << "Display the automata" << std::endl;
  spot::dotty_reachable(std::cout, automata);

  //std::cout << "Initialize the reduction automata" << std::endl;
  automatareduc = new spot::tgba_reduc(automata);

  //aut_red = spot::reduc_tgba_sim(automata);

  //std::cout << "Compute the simulation relation" << std::endl;
  //rel = spot::get_direct_relation_simulation(automatareduc);
  rel = spot::get_delayed_relation_simulation(automatareduc);
  //rel = NULL;

  //std::cout << "Display of the parity game" << std::endl;

  //std::cout << "Display of the simulation relation" << std::endl;
  if (rel != NULL)
    automatareduc->display_rel_sim(rel, std::cout);

  //std::cout << "Prune automata using simulation relation" << std::endl;
  if (rel != NULL)
    automatareduc->prune_automata(rel);

  //automatareduc->compute_scc();
  //std::cout << "Prune automata using scc" << std::endl;
  //automatareduc->prune_scc();

  //std::cout << "Display of scc" << std::endl;
  //automatareduc->display_scc(std::cout);


  if (rel != NULL)
      spot::free_relation_simulation(rel);

  if (automatareduc != NULL)
    {
      std::cout << "Display of the minimize automata" << std::endl;
      std::cout << std::endl;
      spot::dotty_reachable(std::cout, automatareduc);
    }

  if (aut_red != NULL)
    delete aut_red;
  if (automata != NULL)
    delete automata;
  if (automatareduc != NULL)
    delete automatareduc;
  if (f != NULL)
    spot::ltl::destroy(f);

  assert(spot::ltl::atomic_prop::instance_count() == 0);
  assert(spot::ltl::unop::instance_count() == 0);
  assert(spot::ltl::binop::instance_count() == 0);
  assert(spot::ltl::multop::instance_count() == 0);

  if (dict != NULL)
    delete dict;

  return exit_code;
}
