#ifndef SPOT_LTLPARSE_PUBLIC_HH
# define SPOT_LTLPARSE_PUBLIC_HH

# include <string>
# include "ltlast/formulae.hh"
# include "location.hh"
# include <list>
# include <utility>
# include <iostream>

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

    // Return true iff any diagnostic was output to os.
    bool format_parse_errors(std::ostream& os,
			     const std::string& ltl_string,
			     parse_error_list& error_list);
  }
}

#endif // SPOT_LTLPARSE_PUBLIC_HH
