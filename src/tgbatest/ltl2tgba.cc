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

#include <iostream>
#include <cassert>
#include <fstream>
#include <string>
#include "ltlvisit/destroy.hh"
#include "ltlvisit/reduce.hh"
#include "ltlvisit/tostring.hh"
#include "ltlvisit/apcollect.hh"
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
#include "tgbaalgos/reductgba_sim.hh"

void
syntax(char* prog)
{
  std::cerr << "Usage: "<< prog << " [OPTIONS...] formula" << std::endl
            << "       "<< prog << " -F [OPTIONS...] file" << std::endl
            << "       "<< prog << " -X [OPTIONS...] file" << std::endl
	    << std::endl
	    << "Options:" << std::endl
	    << "  -a    display the acceptance_conditions BDD, not the "
	    << "reachability graph"
	    << std::endl
	    << "  -A    same as -a, but as a set" << std::endl
	    << "  -d    turn on traces during parsing" << std::endl
	    << "  -D    degeneralize the automaton" << std::endl
	    << "  -e    emptiness-check (Couvreur), expect and compute "
	    << "a counter-example" << std::endl
	    << "  -e2   emptiness-check (Couvreur variant), expect and compute "
	    << "a counter-example" << std::endl
	    << "  -E    emptiness-check (Couvreur), expect no counter-example "
	    << std::endl
	    << "  -E2   emptiness-check (Couvreur variant), expect no "
	    << "counter-example " << std::endl
            << "  -f    use Couvreur's FM algorithm for translation"
	    << std::endl
            << "  -F    read the formula from the file" << std::endl
            << "  -L    fair-loop approximation (implies -f)" << std::endl
	    << "  -m    magic-search (implies -D), expect a counter-example"
	    << std::endl
	    << "  -M    magic-search (implies -D), expect no counter-example"
	    << std::endl
	    << "  -n    same as -m, but display more counter-examples"
	    << std::endl
	    << "  -N    display the never clain for Spin "
	    << "(implies -D)" << std::endl
            << "  -p    branching postponement (implies -f)" << std::endl
	    << "  -r    display the relation BDD, not the reachability graph"
	    << std::endl
	    << "  -r1   reduce formula using basic rewriting" << std::endl
	    << "  -r2   reduce formula using class of eventuality and "
	    << "and universality" << std::endl
	    << "  -r3   reduce formula using implication between "
	    << "sub-formulae" << std::endl
	    << "  -r4   reduce formula using all rules" << std::endl
	    << "  -rd   display the reduce formula" << std::endl
	    << "  -R    same as -r, but as a set" << std::endl
	    << "  -R1q  use direct simulation to merge some state "
	    << "(use -L for more reduction)"
	    << std::endl
	    << "  -R1t  use direct simulation to remove some transition "
	    << "(use -L for more reduction)"
	    << std::endl
	    << "  -R2q  use delayed simulation to merge some state "
	    << "(the automaton must be degeneralised)"
	    << std::endl
	    << "  -R2t  use delayed simulation to remove some transition "
	    << "(the automaton must be degeneralised)"
	    << std::endl
	    << "  -R3   use SCC to reduce the automata"
	    << std::endl
	    << "  -Rd   display the simulation relation"
	    << std::endl
	    << "  -RD   display the parity game (dot format)"
	    << std::endl
	    << "  -s    convert to explicit automata, and number states "
	    << "in DFS order" << std::endl
	    << "  -S    convert to explicit automata, and number states "
	    << "in BFS order" << std::endl
	    << "  -t    display reachable states in LBTT's format" << std::endl
            << "  -U[PROPS]  consider atomic properties PROPS as exclusive "
	    << "events (implies -f)" << std::endl
	    << "  -v    display the BDD variables used by the automaton"
	    << std::endl
            << "  -x    try to produce a more deterministic automata "
	    << "(implies -f)" << std::endl
	    << "  -X    do not compute an automaton, read it from a file"
	    << std::endl
	    << "  -y    do not merge states with same symbolic representation "
	    << "(implies -f)" << std::endl;
  exit(2);
}

int
main(int argc, char** argv)
{
  int exit_code = 0;

  bool debug_opt = false;
  bool degeneralize_opt = false;
  bool fm_opt = false;
  bool fm_exprop_opt = false;
  bool fm_symb_merge_opt = true;
  bool file_opt = false;
  int output = 0;
  int formula_index = 0;
  enum { None, Couvreur, Couvreur2, MagicSearch } echeck = None;
  enum { NoneDup, BFS, DFS } dupexp = NoneDup;
  bool magic_many = false;
  bool expect_counter_example = false;
  bool from_file = false;
  int reduc_aut = spot::Reduce_None;
  int redopt = spot::ltl::Reduce_None;
  bool display_reduce_form = false;
  bool display_rel_sim = false;
  bool display_parity_game = false;
  bool post_branching = false;
  bool fair_loop_approx = false;
  spot::ltl::environment& env(spot::ltl::default_environment::instance());
  spot::ltl::atomic_prop_set* unobservables = 0;

  for (;;)
    {
      if (argc < formula_index + 2)
	syntax(argv[0]);

      ++formula_index;

      if (!strcmp(argv[formula_index], "-a"))
	{
	  output = 2;
	}
      else if (!strcmp(argv[formula_index], "-A"))
	{
	  output = 4;
	}
      else if (!strcmp(argv[formula_index], "-d"))
	{
	  debug_opt = true;
	}
      else if (!strcmp(argv[formula_index], "-D"))
	{
	  degeneralize_opt = true;
	}
      else if (!strcmp(argv[formula_index], "-e"))
	{
	  echeck = Couvreur;
	  expect_counter_example = true;
	  output = -1;
	}
      else if (!strcmp(argv[formula_index], "-e2"))
	{
	  echeck = Couvreur2;
	  expect_counter_example = true;
	  output = -1;
	}
      else if (!strcmp(argv[formula_index], "-E"))
	{
	  echeck = Couvreur;
	  expect_counter_example = false;
	  output = -1;
	}
      else if (!strcmp(argv[formula_index], "-E2"))
	{
	  echeck = Couvreur2;
	  expect_counter_example = false;
	  output = -1;
	}
      else if (!strcmp(argv[formula_index], "-f"))
	{
	  fm_opt = true;
	}
      else if (!strcmp(argv[formula_index], "-F"))
	{
	  file_opt = true;
	}
      else if (!strcmp(argv[formula_index], "-L"))
	{
	  fair_loop_approx = true;
	  fm_opt = true;
	}
      else if (!strcmp(argv[formula_index], "-m"))
	{
	  echeck = MagicSearch;
	  degeneralize_opt = true;
	  expect_counter_example = true;
	  output = -1;
	}
      else if (!strcmp(argv[formula_index], "-M"))
	{
	  echeck = MagicSearch;
	  degeneralize_opt = true;
	  expect_counter_example = false;
	  output = -1;
	}
      else if (!strcmp(argv[formula_index], "-n"))
	{
	  echeck = MagicSearch;
	  degeneralize_opt = true;
	  expect_counter_example = true;
	  output = -1;
	  magic_many = true;
	}
      else if (!strcmp(argv[formula_index], "-N"))
	{
	  degeneralize_opt = true;
	  output = 8;
	}
      else if (!strcmp(argv[formula_index], "-p"))
	{
	  post_branching = true;
	  fm_opt = true;
	}
      else if (!strcmp(argv[formula_index], "-r"))
	{
	  output = 1;
	}
      else if (!strcmp(argv[formula_index], "-r1"))
	{
	  redopt |= spot::ltl::Reduce_Basics;
	}
      else if (!strcmp(argv[formula_index], "-r2"))
	{
	  redopt |= spot::ltl::Reduce_Eventuality_And_Universality;
	}
      else if (!strcmp(argv[formula_index], "-r3"))
	{
	  redopt |= spot::ltl::Reduce_Syntactic_Implications;
	}
      else if (!strcmp(argv[formula_index], "-r4"))
	{
	  redopt |= spot::ltl::Reduce_All;
	}
      else if (!strcmp(argv[formula_index], "-R"))
	{
	  output = 3;
	}
      else if (!strcmp(argv[formula_index], "-R1q"))
	{
	  reduc_aut |= spot::Reduce_quotient_Dir_Sim;
	}
      else if (!strcmp(argv[formula_index], "-R1t"))
	{
	  reduc_aut |= spot::Reduce_transition_Dir_Sim;
	}
      else if (!strcmp(argv[formula_index], "-R2q"))
	{
	  reduc_aut |= spot::Reduce_quotient_Del_Sim;
	}
      else if (!strcmp(argv[formula_index], "-R2t"))
	{
	  reduc_aut |= spot::Reduce_transition_Del_Sim;
	}
      else if (!strcmp(argv[formula_index], "-R3"))
	{
	  reduc_aut |= spot::Reduce_Scc;
	}
      else if (!strcmp(argv[formula_index], "-rd"))
	{
	  display_reduce_form = true;
	}
      else if (!strcmp(argv[formula_index], "-Rd"))
	{
	  display_rel_sim = true;
	}
      else if (!strcmp(argv[formula_index], "-RD"))
	{
	  display_parity_game = true;
	}
      else if (!strcmp(argv[formula_index], "-s"))
	{
	  dupexp = DFS;
	}
      else if (!strcmp(argv[formula_index], "-S"))
	{
	  dupexp = BFS;
	}
      else if (!strcmp(argv[formula_index], "-t"))
	{
	  output = 6;
	}
      else if (!strncmp(argv[formula_index], "-U", 2))
	{
	  unobservables = new spot::ltl::atomic_prop_set;
	  fm_opt = true;
	  // Parse -U's argument.
	  const char* tok = strtok(argv[formula_index] + 2, ", \t;");
	  while (tok)
	    {
	      unobservables->insert
		(static_cast<spot::ltl::atomic_prop*>(env.require(tok)));
	      tok = strtok(0, ", \t;");
	    }
	}
      else if (!strcmp(argv[formula_index], "-v"))
	{
	  output = 5;
	}
      else if (!strcmp(argv[formula_index], "-x"))
	{
	  fm_opt = true;
	  fm_exprop_opt = true;
	}
      else if (!strcmp(argv[formula_index], "-X"))
	{
	  from_file = true;
	}
      else if (!strcmp(argv[formula_index], "-y"))
	{
	  fm_opt = true;
	  fm_symb_merge_opt = false;
	}
      else
	{
	  break;
	}
    }

  std::string input;

  if (file_opt)
    {
      if (strcmp(argv[formula_index], "-"))
	{
	  std::ifstream fin(argv[formula_index]);
	  if (!fin)
	    {
	      std::cerr << "Cannot open " << argv[formula_index] << std::endl;
	      exit(2);
	    }

	  if (!std::getline(fin, input, '\0'))
	    {
	      std::cerr << "Cannot read " << argv[formula_index] << std::endl;
	      exit(2);
	    }
	}
      else
	{
	  std::getline(std::cin, input, '\0');
	}
    }
  else
    {
      input = argv[formula_index];
    }

  spot::bdd_dict* dict = new spot::bdd_dict();

  spot::ltl::formula* f = 0;
  if (!from_file)
    {
      spot::ltl::parse_error_list pel;
      f = spot::ltl::parse(input, pel, env, debug_opt);
      exit_code = spot::ltl::format_parse_errors(std::cerr, input, pel);
    }
  if (f || from_file)
    {
      spot::tgba_bdd_concrete* concrete = 0;
      spot::tgba* to_free = 0;
      spot::tgba* a = 0;

      if (from_file)
	{
	  spot::tgba_parse_error_list pel;
	  spot::tgba_explicit* e;
	  to_free = a = e = spot::tgba_parse(input, pel, dict, env, debug_opt);
	  if (spot::format_tgba_parse_errors(std::cerr, pel))
	    return 2;
	  e->merge_transitions();
	}
      else
	{
	  if (redopt != spot::ltl::Reduce_None)
	    {
	      spot::ltl::formula* t = spot::ltl::reduce(f, redopt);
	      spot::ltl::destroy(f);
	      f = t;
	      if (display_reduce_form)
		std::cout << spot::ltl::to_string(f) << std::endl;
	    }

	  if (fm_opt)
	    to_free = a = spot::ltl_to_tgba_fm(f, dict, fm_exprop_opt,
					       fm_symb_merge_opt,
					       post_branching,
					       fair_loop_approx, unobservables);
	  else
	    to_free = a = concrete = spot::ltl_to_tgba_lacim(f, dict);
	}

      spot::tgba_tba_proxy* degeneralized = 0;
      if (degeneralize_opt)
	a = degeneralized = new spot::tgba_tba_proxy(a);

      spot::tgba_reduc* aut_red = 0;
      if (reduc_aut != spot::Reduce_None)
	{
	  a = aut_red = new spot::tgba_reduc(a);

	  if (reduc_aut & spot::Reduce_Scc)
	    aut_red->prune_scc();

	  if (reduc_aut & (spot::Reduce_quotient_Dir_Sim |
			   spot::Reduce_transition_Dir_Sim |
			   spot::Reduce_quotient_Del_Sim |
			   spot::Reduce_transition_Del_Sim))
	    {
	      spot::direct_simulation_relation* rel_dir = 0;
	      spot::delayed_simulation_relation* rel_del = 0;

	      if (reduc_aut & (spot::Reduce_quotient_Dir_Sim |
			       spot::Reduce_transition_Dir_Sim))
		rel_dir = spot::get_direct_relation_simulation(a,
							       std::cout,
							       display_parity_game);
	      else if (reduc_aut & (spot::Reduce_quotient_Del_Sim |
				    spot::Reduce_transition_Del_Sim))
		rel_del = spot::get_delayed_relation_simulation(a,
								std::cout,
								display_parity_game);

	      if (display_rel_sim)
		{
		  if (rel_dir)
		    aut_red->display_rel_sim(rel_dir, std::cout);
		  if (rel_del)
		    aut_red->display_rel_sim(rel_del, std::cout);
		}

	      if (reduc_aut & spot::Reduce_quotient_Dir_Sim)
		aut_red->quotient_state(rel_dir);
	      if (reduc_aut & spot::Reduce_transition_Dir_Sim)
		aut_red->delete_transitions(rel_dir);
	      if (reduc_aut & spot::Reduce_quotient_Del_Sim)
		aut_red->quotient_state(rel_del);
	      if (reduc_aut & spot::Reduce_transition_Del_Sim)
		aut_red->delete_transitions(rel_del);

	      if (rel_dir)
		spot::free_relation_simulation(rel_dir);
	      if (rel_del)
		spot::free_relation_simulation(rel_del);
	    }
	}

      spot::tgba_explicit* expl = 0;
      switch (dupexp)
	{
	case NoneDup:
	  break;
	case BFS:
	  a = expl = tgba_dupexp_bfs(a);
	  break;
	case DFS:
	  a = expl = tgba_dupexp_dfs(a);
	  break;
	}

      switch (output)
	{
	case -1:
	  /* No output.  */
	  break;
	case 0:
	  spot::dotty_reachable(std::cout, a);
	  break;
	case 1:
	  if (concrete)
	    spot::bdd_print_dot(std::cout, concrete->get_dict(),
				concrete->get_core_data().relation);
	  break;
	case 2:
	  if (concrete)
	    spot::bdd_print_dot(std::cout, concrete->get_dict(),
				concrete->
				get_core_data().acceptance_conditions);
	  break;
	case 3:
	  if (concrete)
	    spot::bdd_print_set(std::cout, concrete->get_dict(),
				concrete->get_core_data().relation);
	  break;
	case 4:
	  if (concrete)
	    spot::bdd_print_set(std::cout, concrete->get_dict(),
				concrete->
				get_core_data().acceptance_conditions);
	  break;
	case 5:
	  a->get_dict()->dump(std::cout);
	  break;
	case 6:
	  spot::lbtt_reachable(std::cout, a);
	  break;
	case 8:
	  spot::never_claim_reachable(std::cout, degeneralized, f);
	  break;
	default:
	  assert(!"unknown output option");
	}

      spot::emptiness_check* ec = 0;
      spot::tgba* ec_a = 0;
      switch (echeck)
	{
	case None:
	  break;

	case Couvreur:
	  ec_a = a;
	  ec = new spot::couvreur99_check(a);
	  break;

	case Couvreur2:
	  ec_a = a;
	  ec = new spot::couvreur99_check_shy(a);
	  break;

	case MagicSearch:
	  ec_a = degeneralized;
	  ec = new spot::magic_search(degeneralized);
	  break;
	}

      if (ec)
	{
	  do
	    {
	      spot::emptiness_check_result* res = ec->check();
	      if (expect_counter_example != !!res)
		exit_code = 1;

	      if (!res)
		{
		  std::cout << "no accepting run found" << std::endl;
		  break;
		}
	      else
		{

		  spot::tgba_run* run = res->accepting_run();
		  if (!run)
		    {
		      std::cout << "an accepting run exist" << std::endl;
		    }
		  else
		    {
		      spot::print_tgba_run(std::cout, run, ec_a);
		      delete run;
		    }
		}
	      delete res;
	    }
	  while (magic_many);
	  delete ec;
	}

      if (f)
        spot::ltl::destroy(f);
      if (expl)
	delete expl;
      if (degeneralize_opt)
	delete degeneralized;
      if (aut_red)
	delete aut_red;

      delete to_free;
    }
  else
    {
      exit_code = 1;
    }

  assert(spot::ltl::atomic_prop::instance_count() == 0);
  assert(spot::ltl::unop::instance_count() == 0);
  assert(spot::ltl::binop::instance_count() == 0);
  assert(spot::ltl::multop::instance_count() == 0);
  delete dict;
  return exit_code;
}
