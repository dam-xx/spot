#include "public.hh"

namespace spot 
{
  namespace ltl
  {

    bool 
    format_parse_errors(std::ostream& os,
			const std::string& ltl_string,
			parse_error_list& error_list)
    {
      bool printed = false;
      spot::ltl::parse_error_list::iterator it;
      for (it = error_list.begin(); it != error_list.end(); ++it)
	{
	  os << ">>> " << ltl_string << std::endl;
	  yy::Location& l = it->first;

	  unsigned n = 0;
	  for (; n < 4 + l.begin.column; ++n)
	    os << ' ';
	  // Write at least one '^', even if begin==end.
	  os << '^';
	  ++n;
	  for (; n < 4 + l.end.column; ++n)
	    os << '^';
	  os << std::endl << it->second << std::endl << std::endl;
	  printed = true;
	}
      return printed;
    }

  }
}
