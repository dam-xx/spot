
#include <iostream>
#include <cassert>
#include "ltlvisit/destroy.hh"
#include "ltlast/allnodes.hh"
#include "ltlparse/public.hh"
#include "tgbaalgos/ltl2tgba_fm.hh"
#include "tgbaalgos/emptinesscheck.hh"
#include "tgba/bddprint.hh"
//#include "tgba/tgbabddtranslatefactory.hh"
#include "tgbaalgos/dotty.hh"

void
syntax(char* prog)
{ std::cerr << "Usage: "<< prog << " [OPTIONS...] formula" << std::endl
	    << std::endl
	    << "Options:" << std::endl
	    << "  -a   display the accepting_conditions BDD, not the reachability graph"
	    << std::endl
	    << "  -A   same as -a, but as a set" << std::endl
	    << "  -d   turn on traces during parsing" << std::endl
            << "  -c   emptinesschecking + counter example" << std::endl
            << "  -e   emptinesschecking for the automaton" << std::endl
	    << "  -o   re-order BDD variables in the automata" << std::endl
	    << std::endl
	    << "  -r   display the relation BDD, not the reachability graph"
	    << std::endl
	    << "  -R   same as -r, but as a set" << std::endl
	    << "  -v   display the BDD variables used by the automaton"
	    << std::endl;
  exit(2);
}

std::string
print_emptiness_check_ans (bool ans)
{
    if (ans)
      {
	return "EMPTY-LANGAGE";
      }
    else
      {
	return "CONSISTENT-AUTOMATA";
      }
}

int
main(int argc, char** argv)
{
  int exit_code = 0;

  bool debug_opt = false;
  bool defrag_opt = false;
  spot::emptiness_check* empty_check = new spot::emptiness_check();
  bool emptiness = true;
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
      else if (!strcmp(argv[formula_index], "-e"))
	{
	  output = 6;
	}
      else if (!strcmp(argv[formula_index], "-c"))
	{
	  output = 7;
	}
      else if (!strcmp(argv[formula_index], "-o"))
	{
	  defrag_opt = true;
	}
      else if (!strcmp(argv[formula_index], "-r"))
	{
	  output = 1;
	}
      else if (!strcmp(argv[formula_index], "-R"))
	{
	  output = 3;
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

  exit_code = spot::ltl::format_parse_errors(std::cerr, argv[formula_index], pel);
      spot::bdd_dict* dict = new spot::bdd_dict();
  if (f)
    {
      spot::tgba_explicit* a = spot::ltl_to_tgba_fm(f, dict);
      spot::ltl::destroy(f);
      switch (output)
	{
	case 6:
	  emptiness = empty_check->tgba_emptiness_check(a);
	  std::cout << print_emptiness_check_ans(emptiness) << std::endl;
	  break;
	case 7:
	  empty_check->counter_example(a);
     	  break;
	default:
	  assert(!"unknown output option");
	}
      delete a;
      delete empty_check;
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
