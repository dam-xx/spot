#include <iostream>
#include "ltlparse/public.hh"
#include "ltlvisit/dump.hh"
#include "ltlvisit/dotty.hh"

void
syntax(char *prog)
{
  std::cerr << prog << " [-d] formula" << std::endl;
  exit(2);
}

int
main(int argc, char **argv)
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
  spot::ltl::formula *f = spot::ltl::parse(argv[formula_index], 
					   pel, env, debug);

  spot::ltl::parse_error_list::iterator it;
  exit_code = 
    spot::ltl::format_parse_errors(std::cerr, argv[formula_index], pel);

  if (f)
    {
#ifdef DOTTY
      spot::ltl::dotty(*f, std::cout);
#else
      spot::ltl::dump(*f, std::cout);
      std::cout << std::endl;
#endif
      delete f;
    }
  else
    {
      exit_code = 1;
    }
    
  return exit_code;
}
