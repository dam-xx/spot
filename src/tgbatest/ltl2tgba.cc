#include <iostream>
#include <cassert>
#include "ltlvisit/destroy.hh"
#include "ltlast/allnodes.hh"
#include "ltlparse/public.hh"
#include "tgbaalgos/ltl2tgba.hh"
#include "tgba/bddprint.hh"
#include "tgbaalgos/dotty.hh"
#include "tgbaalgos/lbtt.hh"
#include "tgba/tgbatba.hh"

void
syntax(char* prog)
{
  std::cerr << "Usage: "<< prog << " [OPTIONS...] formula" << std::endl
	    << std::endl
	    << "Options:" << std::endl
	    << "  -a   display the accepting_conditions BDD, not the reachability graph"
	    << std::endl
	    << "  -A   same as -a, but as a set" << std::endl
	    << "  -d   turn on traces during parsing" << std::endl
	    << "  -D   degeneralize the automaton" << std::endl
	    << "  -r   display the relation BDD, not the reachability graph"
	    << std::endl
	    << "  -R   same as -r, but as a set" << std::endl
	    << "  -t   display reachable states in LBTT's format" << std::endl
	    << "  -v   display the BDD variables used by the automaton"
	    << std::endl;
  exit(2);
}

int
main(int argc, char** argv)
{
  int exit_code = 0;

  bool debug_opt = false;
  bool degeneralize_opt = false;
  int output = 0;
  int formula_index = 0;

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
      else if (!strcmp(argv[formula_index], "-r"))
	{
	  output = 1;
	}
      else if (!strcmp(argv[formula_index], "-R"))
	{
	  output = 3;
	}
      else if (!strcmp(argv[formula_index], "-t"))
	{
	  output = 6;
	}
      else if (!strcmp(argv[formula_index], "-v"))
	{
	  output = 5;
	}
      else
	{
	  break;
	}
    }

  spot::ltl::environment& env(spot::ltl::default_environment::instance());
  spot::ltl::parse_error_list pel;
  spot::ltl::formula* f = spot::ltl::parse(argv[formula_index],
					   pel, env, debug_opt);

  exit_code =
    spot::ltl::format_parse_errors(std::cerr, argv[formula_index], pel);

  spot::bdd_dict* dict = new spot::bdd_dict();
  if (f)
    {
      spot::tgba_bdd_concrete* concrete = spot::ltl_to_tgba(f, dict);
      spot::tgba* a = concrete;
      spot::ltl::destroy(f);

      spot::tgba* degeneralized = 0;
      if (degeneralize_opt)
	a = degeneralized = new spot::tgba_tba_proxy(a);

      switch (output)
	{
	case 0:
	  spot::dotty_reachable(std::cout, a);
	  break;
	case 1:
	  spot::bdd_print_dot(std::cout, concrete->get_dict(),
			      concrete->get_core_data().relation);
	  break;
	case 2:
	  spot::bdd_print_dot(std::cout, concrete->get_dict(),
			      concrete->get_core_data().accepting_conditions);
	  break;
	case 3:
	  spot::bdd_print_set(std::cout, concrete->get_dict(),
			      concrete->get_core_data().relation);
	  break;
	case 4:
	  spot::bdd_print_set(std::cout, concrete->get_dict(),
			      concrete->get_core_data().accepting_conditions);
	  break;
	case 5:
	  a->get_dict()->dump(std::cout);
	  break;
	case 6:
	  spot::lbtt_reachable(std::cout, a);
	  break;
	default:
	  assert(!"unknown output option");
	}

      if (degeneralize_opt)
	delete degeneralized;

      delete concrete;
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
