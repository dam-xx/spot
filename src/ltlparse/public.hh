#ifndef SPOT_LTLPARSE_PUBLIC_HH
# define SPOT_LTLPARSE_PUBLIC_HH

# include <string>
# include "ltlast/formulae.hh"
# include "location.hh"
# include <list>
# include <utility>

namespace spot 
{
  namespace ltl
  {
    typedef std::pair<yy::Location, std::string> parse_error;
    typedef std::list<parse_error> parse_error_list;

    // Beware: this function is *not* reentrant.
    formulae* parse(const std::string& ltl_string, 
		    parse_error_list& error_list,
		    bool debug = false);
  }
}

#endif // SPOT_LTLPARSE_PUBLIC_HH
