#include <iostream>
#include <sstream>
#include "ltlparse/public.hh"
#include "ltlvisit/tostring.hh"
#include "ltlvisit/equals.hh"

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

  std::ostringstream os;
  spot::ltl::to_string(*f1, os);
  std::cout << os.str() << std::endl;

  spot::ltl::formula* f2 = spot::ltl::parse(os.str(), p1);

  if (spot::ltl::format_parse_errors(std::cerr, os.str(), p1))
    return 2;

  std::ostringstream os2;
  spot::ltl::to_string(*f2, os2);
  std::cout << os2.str() << std::endl;

  if (os2.str() != os.str())
    return 1;

  if (! equals(f1, f2))
    return 1;
}
 
