#include <iostream>
#include "ltlparse/public.hh"
#include "ltlvisit/equals.hh"

void
syntax(char *prog)
{
  std::cerr << prog << " formulae1 formulae2" << std::endl;
  exit(2);
}

bool 
print_parse_error(const char* f, spot::ltl::parse_error_list& pel)
{
  bool err = false;

  spot::ltl::parse_error_list::iterator it;
  for (it = pel.begin(); it != pel.end(); ++it)
    {
      std::cerr << ">>> " << f << std::endl;
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
      err = true;
    }
  return err;
}

int
main(int argc, char **argv)
{
  if (argc != 3)
    syntax(argv[0]);

  
  spot::ltl::parse_error_list p1;
  spot::ltl::formulae *f1 = spot::ltl::parse(argv[1], p1);

  if (print_parse_error(argv[1], p1))
    return 2;

  spot::ltl::parse_error_list p2;
  spot::ltl::formulae *f2 = spot::ltl::parse(argv[2], p2);
			
  if (print_parse_error(argv[2], p2))
    return 2;

  if (equals(f1, f2))
    return 0;
  return 1;

}
