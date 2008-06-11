// Copyright (C) 2008 Laboratoire
// d'Informatique de Paris 6 (LIP6), département Systèmes Répartis
// Coopératifs (SRC), Université Pierre et Marie Curie.
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

#include <cstdlib>
#include <iostream>
#include <cassert>
#include <cstring>
#include "common.hh"
#include "nips.hh"
#include "tgbaalgos/gtec/gtec.hh"
#include "tgbaalgos/gtec/ce.hh"
#include "tgbaalgos/projrun.hh"

void
display_stats(const spot::unsigned_statistics* s)
{
  assert(s);
  spot::unsigned_statistics::stats_map::const_iterator i;
  for (i = s->stats.begin(); i != s->stats.end(); ++i)
    std::cout << i->first << " = " << (s->*i->second)() << std::endl;
}

int
main(int argc, char **argv)
  try
  {
    // enum {Couvreur, Couvreur2} check = Couvreur;
    bool compute_counter_example = true;

    if (argc < 2)
    {
      std::cerr << "usage: " << argv[0] << "[OPTIONS...] promela_bytecode"
		<< std::endl
		<< "with OPTIONS :" << std::endl
		<< "  -c  compute an example" << std::endl
		<< "      (instead of just checking for emptiness)" << std::endl
		<< "  -eALGO  use ALGO emptiness-check (default)" << std::endl
		<< "Where ALGO should be one of:" << std::endl
		<< "  Cou99(OPTIONS) (the default)" << std::endl
		<< "  CVWY90(OPTIONS)" << std::endl
		<< "  GV04(OPTIONS)" << std::endl
		<< "  SE05(OPTIONS)" << std::endl
		<< "  Tau03(OPTIONS)" << std::endl
		<< "  Tau03_opt(OPTIONS)" << std::endl;

      exit(2);
    }

    int arg_index = 1;
    spot::emptiness_check_instantiator* echeck_inst = 0;
    const char* echeck_algo = "Cou99";

    for (; arg_index < argc - 1; ++arg_index)
    {
      if (!strcmp(argv[arg_index], "-c"))
	compute_counter_example = false;
      else if (!strncmp(argv[arg_index], "-e", 2))
      {
	echeck_algo = 2 + argv[arg_index];
	if (!*echeck_algo)
	  echeck_algo = "Cou99";
      }
    }

    const char* err;
    echeck_inst =
      spot::emptiness_check_instantiator::construct(echeck_algo, &err);
    if (!echeck_inst)
    {
      std::cerr << "Failed to parse argument of -e near `"
		<< err <<  "'" << std::endl;
      exit(2);
    }

    spot::bdd_dict* dict = new spot::bdd_dict();
    spot::nips_interface nips(dict, argv[arg_index]);
    spot::tgba* a = nips.automaton();

    spot::emptiness_check* ec = echeck_inst->instantiate(a);
    spot::emptiness_check_result* res = ec->check();

    if (res)
    {
      if (compute_counter_example)
      {
	spot::tgba_run* run = res->accepting_run();
	spot::print_tgba_run(std::cout, a, run);
	std::cout << "non empty" << std::endl;
	ec->print_stats(std::cout);
	delete run;
      }
      else
      {
	std::cout << "non empty" << std::endl;
	ec->print_stats(std::cout);
      }
      delete res;
    }
    else
    {
      std::cout << "empty" << std::endl;
      ec->print_stats(std::cout);
    }
    std::cout << std::endl;
    delete ec;
  }
  catch (spot::nips_exception& e)
  {
    std::cerr << e << std::endl;
    return 1;
  }
