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
/// \brief LTL formula, AST and visitor interface
#ifndef SPOT_LTLAST_FORMULA_HH
# define SPOT_LTLAST_FORMULA_HH

# include "internal/formula.hh"
# include "internal/atomic_prop.hh"
# include "internal/constant.hh"
# include "internal/unop.hh"
# include "internal/binop.hh"
# include "internal/multop.hh"

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

    struct ltl_t;
    struct visitor;
    struct const_visitor;

    /// \brief An LTL formula.
    /// \ingroup ltl_essential
    /// \ingroup ltl_ast
    ///
    /// The only way you can work with a formula is to
    /// build a spot::ltl::visitor or spot::ltl::const_visitor.
    typedef spot::internal::formula<ltl_t> formula;

    /// Forward declarations
    formula* clone(const formula* f);
    std::ostream& to_string(const formula* f, std::ostream& os);
    void destroy(const formula* f);

    struct ltl_t
    {
      typedef spot::ltl::visitor visitor;
      typedef spot::ltl::const_visitor const_visitor;

      static formula* clone_(const formula* f)
      {
	return clone(f);
      }

      static std::ostream& to_string_(const formula* f, std::ostream& os)
      {
	return to_string(f, os);
      }

      static void destroy_(const formula* f)
      {
	destroy(f);
      }

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

    typedef spot::internal::formula_ptr_less_than formula_ptr_less_than;
    typedef spot::internal::formula_ptr_hash formula_ptr_hash;

    /// \brief Atomic propositions.
    /// \ingroup ltl_ast
    typedef spot::internal::atomic_prop<ltl_t> atomic_prop;

    /// \brief A constant (True or False)
    /// \ingroup ltl_ast
    typedef spot::internal::constant<ltl_t> constant;

    /// \brief Unary operators.
    /// \ingroup ltl_ast
    typedef spot::internal::unop<ltl_t> unop;

    /// \brief Binary operator.
    /// \ingroup ltl_ast
    typedef spot::internal::binop<ltl_t> binop;

    /// \brief Multi-operand operators.
    /// \ingroup ltl_ast
    ///
    /// These operators are considered commutative and associative.
    typedef spot::internal::multop<ltl_t> multop;


    /// \brief Formula visitor that can modify the formula.
    /// \ingroup ltl_essential
    ///
    /// Writing visitors is the prefered way
    /// to traverse a formula, since it doesn't
    /// involve any cast.
    ///
    /// If you do not need to modify the visited formula, inherit from
    /// spot::ltl:const_visitor instead.
    struct visitor
    {
      virtual ~visitor() {}
      virtual void visit(atomic_prop* node) = 0;
      virtual void visit(constant* node) = 0;
      virtual void visit(binop* node) = 0;
      virtual void visit(unop* node) = 0;
      virtual void visit(multop* node) = 0;
    };

    /// \brief Formula visitor that cannot modify the formula.
    ///
    /// Writing visitors is the prefered way
    /// to traverse a formula, since it doesn't
    /// involve any cast.
    ///
    /// If you want to modify the visited formula, inherit from
    /// spot::ltl:visitor instead.
    struct const_visitor
    {
      virtual ~const_visitor() {}
      virtual void visit(const atomic_prop* node) = 0;
      virtual void visit(const constant* node) = 0;
      virtual void visit(const binop* node) = 0;
      virtual void visit(const unop* node) = 0;
      virtual void visit(const multop* node) = 0;
    };
  }
}

#endif // SPOT_LTLAST_FORMULA_HH
