#include <iostream>
#include "ltlparse/public.hh"
#include "ltlvisit/tostring.hh"
#include "ltlvisit/destroy.hh"
#include "ltlast/allnodes.hh"

void
syntax(char *prog)
{
  std::cerr << prog << " formula1" << std::endl;
  exit(2);
}

int
main(int argc, char **argv)
{
  if (argc != 2)
    syntax(argv[0]);

  spot::ltl::parse_error_list p1;
  spot::ltl::formula* f1 = spot::ltl::parse(argv[1], p1);

  if (spot::ltl::format_parse_errors(std::cerr, argv[1], p1))
    return 2;

  // The string generated from an abstract tree should be parsable
  // again.

  std::string f1s = spot::ltl::to_string(*f1);
  std::cout << f1s << std::endl;

  spot::ltl::formula* f2 = spot::ltl::parse(f1s, p1);

  if (spot::ltl::format_parse_errors(std::cerr, f1s, p1))
    return 2;

  // This second abstract tree should be equal to the first.

  if (f1 != f2)
    return 1;

  // It should also map to the same string.

  std::string f2s = spot::ltl::to_string(*f2);
  std::cout << f2s << std::endl;

  if (f2s != f1s)
    return 1;

  spot::ltl::destroy(f1);
  spot::ltl::destroy(f2);
  assert(spot::ltl::atomic_prop::instance_count() == 0);
  assert(spot::ltl::unop::instance_count() == 0);
  assert(spot::ltl::binop::instance_count() == 0);
  assert(spot::ltl::multop::instance_count() == 0);
  return 0;
}
