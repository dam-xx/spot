#include <iostream>
#include "ltlparse/public.hh"
#include "ltlvisit/dump.hh"
#include "ltlvisit/dotty.hh"

void
syntax(char *prog)
{
  std::cerr << prog << " [-d] formulae" << std::endl;
  exit(2);
}

int
main(int argc, char **argv)
{
  int exit_code = 0;

  if (argc < 2)
    syntax(argv[0]);

  bool debug = false;
  int formulae_index = 1;

  if (!strcmp(argv[1], "-d"))
    {
      debug = true;
      if (argc < 3)
	syntax(argv[0]);
      formulae_index = 2;
    }    
  
  spot::ltl::parse_error_list pel;
  spot::ltl::formulae *f = spot::ltl::parse(argv[formulae_index], 
					    pel, debug);

  spot::ltl::parse_error_list::iterator it;
  for (it = pel.begin(); it != pel.end(); ++it)
    {
      std::cerr << ">>> " << argv[formulae_index] << std::endl;
      unsigned n = 0;
      yy::Location& l = it->first;
      for (; n < 4 + l.begin.column; ++n)
	std::cerr << ' ';
      // Write at least one '^', even if begin==end.
      std::cerr << '^';
      ++n;
      for (; n < 4 + l.end.column; ++n)
	std::cerr << '^';
      std::cerr << std::endl << it->second << std::endl << std::endl;
      exit_code = 1;
    }

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
