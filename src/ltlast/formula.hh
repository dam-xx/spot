// Copyright (C) 2003, 2004, 2005, 2008 Laboratoire d'Informatique de
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

/// \file ltlast/formula.hh
/// \brief LTL formula interface
#ifndef SPOT_LTLAST_FORMULA_HH
# define SPOT_LTLAST_FORMULA_HH

# include "internal/formula.hh"

namespace spot
{
  namespace ltl
  {
    /// \defgroup ltl LTL formulae
    ///
    /// This module gathers types and definitions related to LTL formulae.

    /// \addtogroup ltl_essential Essential LTL types
    /// \ingroup ltl

    /// \addtogroup ltl_ast LTL Abstract Syntax Tree
    /// \ingroup ltl

    /// \addtogroup ltl_environment LTL environments
    /// \ingroup ltl
    /// LTL environment implementations.

    /// \addtogroup ltl_algorithm Algorithms for LTL formulae
    /// \ingroup ltl

    /// \addtogroup ltl_io Input/Output of LTL formulae
    /// \ingroup ltl_algorithm

    /// \addtogroup ltl_visitor Derivable visitors
    /// \ingroup ltl_algorithm

    /// \addtogroup ltl_rewriting Rewriting LTL formulae
    /// \ingroup ltl_algorithm

    /// \addtogroup ltl_misc Miscellaneous algorithms for LTL formulae
    /// \ingroup ltl_algorithm

    struct visitor;
    struct const_visitor;

    struct ltl_t
    {
      typedef spot::ltl::visitor visitor;
      typedef spot::ltl::const_visitor const_visitor;

      enum binop { Xor, Implies, Equiv, U, R };
      const char* binop_name(binop op) const
      {
	switch (op)
      	{
	  case Xor:
	    return "Xor";
	  case Implies:
	    return "Implies";
	  case Equiv:
	    return "Equiv";
	  case U:
	    return "U";
	  case R:
	    return "R";
      	}
	// Unreachable code.
	assert(0);
	return 0;
      }

      enum unop { Not, X, F, G };
      const char* unop_name(unop op) const
      {
	switch (op)
	{
	  case Not:
	    return "Not";
	  case X:
	    return "X";
	  case F:
	    return "F";
	  case G:
	    return "G";
	}
	// Unreachable code.
	assert(0);
	return 0;
      }
    };

    /// \brief An LTL formula.
    /// \ingroup ltl_essential
    /// \ingroup ltl_ast
    ///
    /// The only way you can work with a formula is to
    /// build a spot::ltl::visitor or spot::ltl::const_visitor.
    typedef spot::internal::formula<ltl_t> formula;

    typedef spot::internal::formula_ptr_less_than<ltl_t> formula_ptr_less_than;
    typedef spot::internal::formula_ptr_hash<ltl_t> formula_ptr_hash;
  }
}

#endif // SPOT_LTLAST_FORMULA_HH
