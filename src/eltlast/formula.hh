// Copyright (C) 2008 Laboratoire d'Informatique de Paris 6 (LIP6),
// département Systèmes Répartis Coopératifs (SRC), Université Pierre
// et Marie Curie.
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

/// \file eltlast/formula.hh
/// \brief ELTL formula interface
#ifndef SPOT_ELTLAST_FORMULA_HH
# define SPOT_ELTLAST_FORMULA_HH

# include "internal/formula.hh"

namespace spot
{
  namespace eltl
  {
    /// \defgroup eltl ELTL formulae
    ///
    /// This module gathers types and definitions related to ELTL formulae.

    /// \addtogroup eltl_essential Essential ELTL types
    /// \ingroup eltl

    /// \addtogroup eltl_ast ELTL Abstract Syntax Tree
    /// \ingroup eltl

    /// Forward declarations
    struct visitor;
    struct const_visitor;

    /// \brief An ELTL formula.
    /// \ingroup eltl_essential
    /// \ingroup eltl_ast
    ///
    /// The only way you can work with a formula is to
    /// build a spot::eltl::visitor or spot::eltl::const_visitor.
    struct eltl_t
    {
      typedef spot::eltl::visitor visitor;
      typedef spot::eltl::const_visitor const_visitor;

      enum binop { Xor, Implies, Equiv };
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
      	}
	// Unreachable code.
	assert(0);
	return 0;
      }

      enum unop { Not };
      const char* unop_name(unop op) const
      {
	switch (op)
	{
	  case Not:
	    return "Not";
	}
	// Unreachable code.
	assert(0);
	return 0;
      }
    };

    typedef spot::internal::formula<eltl_t> formula;

    typedef spot::internal::formula_ptr_less_than<eltl_t> formula_ptr_less_than;
    typedef spot::internal::formula_ptr_hash<eltl_t> formula_ptr_hash;
  }
}

#endif /* !SPOT_ELTLAST_FORMULA_HH_ */
