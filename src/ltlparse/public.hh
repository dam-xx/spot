#ifndef SPOT_LTLPARSE_PUBLIC_HH
# define SPOT_LTLPARSE_PUBLIC_HH

# include "ltlast/formula.hh"
# include "location.hh"
# include "ltlenv/defaultenv.hh"
# include <string>
# include <list>
# include <utility>
# include <iostream>

namespace spot 
{
  namespace ltl
  {
    /// \brief A parse diagnostic with its location.
    typedef std::pair<yy::Location, std::string> parse_error;
    /// \brief A list of parser diagnostics, as filled by parse.
    typedef std::list<parse_error> parse_error_list;

    /// \brief Build a formula from an LTL string.
    /// \param ltl_string The string to parse.
    /// \param error_list A list that will be filled with
    ///        parse errors that occured during parsing.
    /// \param env The environment into which parsing should take place.
    /// \param debug When true, causes the parser to trace its execution.
    /// \return A pointer to the formula built from \a ltl_string, or
    ///        0 if the input was unparsable.
    ///
    /// Note that the parser usually tries to recover from errors.  It can
    /// return an non zero value even if it encountered error during the
    /// parsing of \a ltl_string.  If you want to make sure \a ltl_string
    /// was parsed succesfully, check \a error_list for emptiness.
    ///
    /// \warning This function is not reentrant.
    formula* parse(const std::string& ltl_string, 
		   parse_error_list& error_list,
		   environment& env = default_environment::instance(),
		   bool debug = false);

    
    /// \brief Format diagnostics produced by spot::ltl::parse.
    /// \param os Where diagnostics should be output.
    /// \param ltl_string The string that were parsed.
    /// \param error_list The error list filled by spot::ltl::parse while
    ///        parsing \a ltl_string.
    /// \return \c true iff any diagnostic was output.
    bool format_parse_errors(std::ostream& os,
			     const std::string& ltl_string,
			     parse_error_list& error_list);
  }
}

#endif // SPOT_LTLPARSE_PUBLIC_HH
