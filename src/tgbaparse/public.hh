#ifndef SPOT_TGBAPARSE_PUBLIC_HH
# define SPOT_TGBAPARSE_PUBLIC_HH

# include "tgba/tgbaexplicit.hh"
# include "location.hh"
# include "ltlenv/defaultenv.hh"
# include <string>
# include <list>
# include <utility>
# include <iostream>

namespace spot
{
  /// \brief A parse diagnostic with its location.
  typedef std::pair<yy::Location, std::string> tgba_parse_error;
  /// \brief A list of parser diagnostics, as filled by parse.
  typedef std::list<tgba_parse_error> tgba_parse_error_list;

  /// \brief Build a spot::tgba_explicit from a text file.
  /// \param filename The name of the file to parse.
  /// \param error_list A list that will be filled with
  ///        parse errors that occured during parsing.
  /// \param env The environment into which parsing should take place.
  /// \param debug When true, causes the parser to trace its execution.
  /// \return A pointer to the tgba built from \a filename, or
  ///        0 if the file could not be opened.
  ///
  /// Note that the parser usually tries to recover from errors.  It can
  /// return an non zero value even if it encountered error during the
  /// parsing of \a filename.  If you want to make sure \a filename
  /// was parsed succesfully, check \a error_list for emptiness.
  ///
  /// \warning This function is not reentrant.
  tgba_explicit* tgba_parse(const std::string& filename,
			    tgba_parse_error_list& error_list,
			    ltl::environment& env
			    = ltl::default_environment::instance(),
			    bool debug = false);

  /// \brief Format diagnostics produced by spot::tgba_parse.
  /// \param os Where diagnostics should be output.
  /// \param error_list The error list filled by spot::ltl::parse while
  ///        parsing \a ltl_string.
  /// \return \c true iff any diagnostic was output.
  bool format_tgba_parse_errors(std::ostream& os,
				tgba_parse_error_list& error_list);
}

#endif // SPOT_TGBAPARSE_PUBLIC_HH