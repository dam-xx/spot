// Copyright (C) 2006 Laboratoire d'Informatique de
// Paris 6 (LIP6), département Systèmes Répartis Coopératifs (SRC),
// Université Pierre et Marie Curie.
//
// This file is part of Spot, a model checking library.
//
// Spot is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Spot is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Spot; see the file COPYING.  If not, write to the Free
// Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#ifndef SPOT_SAUTPARSE_PUBLIC_HH
# define SPOT_SAUTPARSE_PUBLIC_HH

# include "ltlenv/defaultenv.hh"
# undef BISON_LOCATION_HH
# undef BISON_POSITION_HH
# include "sautparse/location.hh"
# include "saut/saut.hh"
# include <string>
# include <list>
# include <utility>
# include <iosfwd>
# include <map>
# include "saut/sync.hh"

namespace spot
{
  /// \addtogroup saut_io
  /// @{

  struct saut_parse_result
  {
    typedef std::map<std::string, spot::saut*> aut_map;
    typedef std::map<std::string, spot::sync*> sync_map;
    aut_map auts;
    sync_map syns;
    ~saut_parse_result();
  };

  /// \brief A parse diagnostic with its location.
  typedef std::pair<sautyy::location, std::string> saut_parse_error;
  /// \brief A list of parser diagnostics, as filled by parse.
  typedef std::list<saut_parse_error> saut_parse_error_list;

  /// \brief Build a spot::saut_explicit from a text file.
  /// \param filename The name of the file to parse.
  /// \param error_list A list that will be filled with
  ///        parse errors that occured during parsing.
  /// \param dict The BDD dictionary where to use.
  /// \param env The environment of atomic proposition into which parsing
  ///        should take place.
  /// \param debug When true, causes the parser to trace its execution.
  /// \return A newly allocated saut_parse_result structure, or
  ///        0 if the file could not be opened.
  ///
  /// Note that the parser usually tries to recover from errors.  It can
  /// return an non zero value even if it encountered error during the
  /// parsing of \a filename.  If you want to make sure \a filename
  /// was parsed succesfully, check \a error_list for emptiness.
  ///
  /// \warning This function is not reentrant.
  saut_parse_result* saut_parse(const std::string& filename,
				saut_parse_error_list& error_list,
				bdd_dict* dict,
				ltl::environment& env
				= ltl::default_environment::instance(),
				bool debug = false);

  /// \brief Format diagnostics produced by spot::saut_parse.
  /// \param os Where diagnostics should be output.
  /// \param filename The filename that should appear in the diagnostics.
  /// \param error_list The error list filled by spot::ltl::parse while
  ///        parsing \a ltl_string.
  /// \return \c true iff any diagnostic was output.
  bool format_saut_parse_errors(std::ostream& os,
				const std::string& filename,
				saut_parse_error_list& error_list);

  /// @}
}

#endif // SPOT_SAUTPARSE_PUBLIC_HH
