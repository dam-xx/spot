#include <iostream>
#include <cassert>
#include "ltlvisit/destroy.hh"
#include "ltlast/allnodes.hh"
#include "ltlparse/public.hh"
#include "tgbaalgos/ltl2tgba.hh"
#include "tgbaalgos/magic.hh"

void
syntax(char* prog)
{
  std::cerr << prog << " formula" << std::endl;
  exit(2);
}

int
main(int argc, char** argv)
{
  int exit_code = 0;
  int formula_index = 0;
  bool all_opt = false;

  for (;;)
    {
      if (argc < formula_index + 1)
	syntax(argv[0]);

      ++formula_index;

      if (!strcmp(argv[formula_index], "-a"))
	{
	  all_opt = true;
	}
      else
	{
	  break;
	}
    }

  spot::ltl::environment& env(spot::ltl::default_environment::instance());

  spot::ltl::parse_error_list pel1;
  spot::ltl::formula* f1 = spot::ltl::parse(argv[formula_index], pel1, env);

  if (spot::ltl::format_parse_errors(std::cerr, argv[formula_index], pel1))
    return 2;

  spot::bdd_dict* dict = new spot::bdd_dict();
  {
    spot::tgba_bdd_concrete* a1 = spot::ltl_to_tgba(f1, dict);
    spot::tgba_tba_proxy* a2 = new spot::tgba_tba_proxy(a1);
    spot::ltl::destroy(f1);

    spot::magic_search ms(a2);
    
    if (ms.check())
      {
	do 
	  ms.print_result (std::cout);
	while (all_opt && ms.check());
      }
    else
      {
	exit_code = 1;
      }

    delete a2;
    delete a1;
  }

  assert(spot::ltl::atomic_prop::instance_count() == 0);
  assert(spot::ltl::unop::instance_count() == 0);
  assert(spot::ltl::binop::instance_count() == 0);
  assert(spot::ltl::multop::instance_count() == 0);
  delete dict;
  return exit_code;
}
