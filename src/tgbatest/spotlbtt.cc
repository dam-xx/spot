#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include "ltlvisit/destroy.hh"
#include "ltlast/allnodes.hh"
#include "ltlparse/public.hh"
#include "tgbaalgos/ltl2tgba.hh"
#include "tgbaalgos/lbtt.hh"
#include "tgba/tgbatba.hh"

void
syntax(char* prog)
{
  std::cerr << "Usage: "<< prog << " file" << std::endl;
  exit(2);
}

int
main(int argc, char** argv)
{
  int exit_code = 0;

  if (argc !=  2)
    syntax(argv[0]);

  std::ifstream fin(argv[1]);
  if (! fin)
    {
      std::cerr << "Cannot open " << argv[1] << std::endl;
      exit(2);
    }

  std::string input;
  if (! std::getline(fin, input, '\0'))
    {
      std::cerr << "Cannot read " << argv[1] << std::endl;
      exit(2);
    }

  spot::bdd_dict* dict = new spot::bdd_dict();

  spot::ltl::environment& env(spot::ltl::default_environment::instance());
  spot::ltl::parse_error_list pel;
  spot::ltl::formula* f = spot::ltl::parse(input, pel, env);

  exit_code = spot::ltl::format_parse_errors(std::cerr, input, pel);

  if (f)
    {
      spot::tgba* a;
      spot::tgba* c = a = spot::ltl_to_tgba(f, dict);
      spot::ltl::destroy(f);
#ifdef TBA
      spot::tgba* d = a = new spot::tgba_tba_proxy(a);
#endif
      spot::lbtt_reachable(std::cout, a);
#ifdef TBA
      delete d;
#endif
      delete c;
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
