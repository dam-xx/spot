#include <iostream>
#include <cassert>
#include "ltlvisit/destroy.hh"
#include "ltlast/allnodes.hh"
#include "ltlparse/public.hh"
#include "tgba/ltl2tgba.hh"
#include "tgbaalgos/dotty.hh"

void
syntax(char* prog)
{
  std::cerr << prog << " [-d] formula" << std::endl;
  exit(2);
}

int
main(int argc, char** argv)
{
  int exit_code = 0;

  if (argc < 2)
    syntax(argv[0]);

  bool debug = false;
  int formula_index = 1;

  if (!strcmp(argv[1], "-d"))
    {
      debug = true;
      if (argc < 3)
	syntax(argv[0]);
      formula_index = 2;
    }

  spot::ltl::environment& env(spot::ltl::default_environment::instance());
  spot::ltl::parse_error_list pel;
  spot::ltl::formula* f = spot::ltl::parse(argv[formula_index],
					   pel, env, debug);

  exit_code =
    spot::ltl::format_parse_errors(std::cerr, argv[formula_index], pel);

  if (f)
    {
      spot::tgba_bdd_concrete a = spot::ltl_to_tgba(f);
      spot::ltl::destroy(f);
      spot::dotty_reachable(std::cout, a);
    }
  else
    {
      exit_code = 1;
    }

  assert(spot::ltl::atomic_prop::instance_count() == 0);
  assert(spot::ltl::unop::instance_count() == 0);
  assert(spot::ltl::binop::instance_count() == 0);
  assert(spot::ltl::multop::instance_count() == 0);
  return exit_code;
}
