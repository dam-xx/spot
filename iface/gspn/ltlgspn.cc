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

#ifndef EESRG
#include "gspn.hh"
#define MIN_ARG 3
#else
#include "eesrg.hh"
#define MIN_ARG 4
#include "tgba/tgbaexplicit.hh"
#include "tgbaparse/public.hh"
#endif
#include "ltlparse/public.hh"
#include "ltlvisit/destroy.hh"
#include "tgba/tgbatba.hh"
#include "tgba/tgbaproduct.hh"
#include "tgbaalgos/ltl2tgba_lacim.hh"
#include "tgbaalgos/ltl2tgba_fm.hh"
#include "tgbaalgos/magic.hh"
#include "tgbaalgos/emptinesscheck.hh"


void
syntax(char* prog)
{
  std::cerr << "Usage: "<< prog
#ifndef EESRG
	    << " [OPTIONS...] model formula props..."   << std::endl
#else
	    << " [OPTIONS...] model formula automata props..."   << std::endl
#endif
	    << std::endl
	    << "  -c  compute an example" << std::endl
	    << "      (instead of just checking for emptiness)" << std::endl
	    << std::endl
	    << "  -e  use Couvreur's emptiness-check (default)" << std::endl
	    << "  -e2 use Couvreur's emptiness-check variant" << std::endl
	    << "  -m  degeneralize and perform a magic-search" << std::endl
	    << std::endl
            << "  -l  use Couvreur's LaCIM algorithm for translation (default)"
	    << std::endl
            << "  -f  use Couvreur's FM algorithm for translation" << std::endl
	    << "  -P  do not project example on model" << std::endl;
  exit(2);
}

int
main(int argc, char **argv)
  try
    {
      int formula_index = 1;
      enum { Couvreur, Couvreur2, Magic } check = Couvreur;
      enum { Lacim, Fm } trans = Lacim;
      bool compute_counter_example = false;
      bool proj = true;

      spot::gspn_environment env;

      while (formula_index < argc && *argv[formula_index] == '-')
	{
	  if (!strcmp(argv[formula_index], "-c"))
	    {
	      compute_counter_example = true;
	    }
	  else if (!strcmp(argv[formula_index], "-e"))
	    {
	      check = Couvreur;
	    }
	  else if (!strcmp(argv[formula_index], "-e2"))
	    {
	      check = Couvreur2;
	    }
	  else if (!strcmp(argv[formula_index], "-m"))
	    {
	      check = Magic;
	    }
	  else if (!strcmp(argv[formula_index], "-l"))
	    {
	      trans = Lacim;
	    }
	  else if (!strcmp(argv[formula_index], "-f"))
	    {
	      trans = Fm;
	    }
	  else if (!strcmp(argv[formula_index], "-P"))
	    {
	      proj = 0;
	    }
	  else
	    {
	      syntax(argv[0]);
	    }
	  ++formula_index;
	}
      if (argc < formula_index + MIN_ARG)
	syntax(argv[0]);


      while (argc >= formula_index + MIN_ARG)
	{
	  env.declare(argv[argc - 1]);
	  --argc;
	}

      spot::ltl::parse_error_list pel;
      spot::ltl::formula* f = spot::ltl::parse(argv[formula_index + 1],
					       pel, env);

      if (spot::ltl::format_parse_errors(std::cerr,
					 argv[formula_index + 1], pel))
	exit(2);

      argv[1] = argv[formula_index];
      spot::bdd_dict* dict = new spot::bdd_dict();

#if EESRG
      spot::gspn_eesrg_interface gspn(2, argv, dict, env);

      spot::tgba_parse_error_list pel1;
      spot::tgba_explicit* control = spot::tgba_parse(argv[formula_index + 2],
						      pel1, dict, env);
      if (spot::format_tgba_parse_errors(std::cerr, pel1))
	return 2;
#else
      spot::gspn_interface gspn(2, argv, dict, env);
#endif

      spot::tgba* a_f = 0;
      switch (trans)
	{
	case Fm:
	  a_f = spot::ltl_to_tgba_fm(f, dict);
	  break;
	case Lacim:
	  a_f = spot::ltl_to_tgba_lacim(f, dict);
	  break;
	}
      spot::ltl::destroy(f);

#ifndef EESRG
      spot::tgba* model        = gspn.automaton();
      spot::tgba_product* prod = new spot::tgba_product(model, a_f);
#else
      spot::tgba_product* ca = new spot::tgba_product(control, a_f);
      spot::tgba* model      = gspn.automaton(ca);
      spot::tgba* prod = model;
#endif

      switch (check)
	{
	case Couvreur:
	case Couvreur2:
	  {
	    spot::emptiness_check ec(prod);
	    bool res;
	    if (check == Couvreur)
	      res = ec.check();
	    else
	      res = ec.check2();
	    const spot::emptiness_check_status* ecs = ec.result();
	    if (!res)
	      {
		if (compute_counter_example)
		  {
		    spot::counter_example ce(ecs);
		    ce.print_result(std::cout, proj ? model : 0);
		    ce.print_stats(std::cout);
		  }
		else
		  {
		    std::cout << "non empty" << std::endl;
		    ecs->print_stats(std::cout);
		  }
	      }
	    else
	      {
		std::cout << "empty" << std::endl;
		ecs->print_stats(std::cout);
	      }
	    std::cout << std::endl;
	    if (!res)
	      exit(1);
	  }
	  break;
	case Magic:
	  {
	    spot::tgba_tba_proxy* d  = new spot::tgba_tba_proxy(prod);
	    spot::magic_search ms(d);

	    if (ms.check())
	      {
		if (compute_counter_example)
		  ms.print_result (std::cout, proj ? model : 0);
		else
		  std::cout << "non-empty" << std::endl;
		exit(1);
	      }
	    else
	      {
		std::cout << "empty" << std::endl;
	      }
	    delete d;
	  }
	}
#ifndef EESRG
      delete prod;
      delete model;
#else
      delete model;
      delete control;
#endif
      delete a_f;
      delete dict;
    }
  catch (spot::gspn_exeption e)
    {
      std::cerr << e << std::endl;
      throw;
    }
