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
# include "internal/atomic_prop.hh"
# include "internal/constant.hh"
# include "internal/unop.hh"
# include "internal/binop.hh"
# include "internal/multop.hh"

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

    /// \addtogroup eltl_environment ELTL environments
    /// \ingroup eltl
    /// ELTL environment implementations.

    /// \addtogroup eltl_algorithm Algorithms for ELTL formulae
    /// \ingroup eltl

    /// \addtogroup eltl_io Input/Output of ELTL formulae
    /// \ingroup eltl_algorithm

    /// \addtogroup eltl_visitor Derivable visitors
    /// \ingroup eltl_algorithm

    /// Forward declarations
    struct eltl_t;
    struct visitor;
    struct const_visitor;

    /// \brief An ELTL formula.
    /// \ingroup eltl_essential
    /// \ingroup eltl_ast
    ///
    /// The only way you can work with a formula is to
    /// build a spot::eltl::visitor or spot::eltl::const_visitor.
    typedef spot::internal::formula<eltl_t> formula;

    /// Forward declarations
    formula* clone(const formula* f);
    std::ostream& to_string(const formula* f, std::ostream& os);
    void destroy(const formula* f);

    struct eltl_t
    {
      typedef spot::eltl::visitor visitor;
      typedef spot::eltl::const_visitor const_visitor;

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

    typedef spot::internal::formula_ptr_less_than formula_ptr_less_than;
    typedef spot::internal::formula_ptr_hash formula_ptr_hash;

    /// \brief Atomic propositions.
    /// \ingroup eltl_ast
    typedef spot::internal::atomic_prop<eltl_t> atomic_prop;

    /// \brief A constant (True or False)
    /// \ingroup eltl_ast
    typedef spot::internal::constant<eltl_t> constant;

    /// \brief Unary operators.
    /// \ingroup eltl_ast
    typedef spot::internal::unop<eltl_t> unop;

    /// \brief Binary operator.
    /// \ingroup eltl_ast
    typedef spot::internal::binop<eltl_t> binop;

    /// \brief Multi-operand operators.
    /// \ingroup eltl_ast
    ///
    /// These operators are considered commutative and associative.
    typedef spot::internal::multop<eltl_t> multop;

    // Forward declaration.
    struct automatop;

    /// \brief Formula visitor that can modify the formula.
    /// \ingroup eltl_essential
    ///
    /// Writing visitors is the prefered way
    /// to traverse a formula, since it doesn't
    /// involve any cast.
    ///
    /// If you do not need to modify the visited formula, inherit from
    /// spot::eltl:const_visitor instead.
    struct visitor
    {
      virtual ~visitor() {}
      virtual void visit(atomic_prop* node) = 0;
      virtual void visit(constant* node) = 0;
      virtual void visit(binop* node) = 0;
      virtual void visit(unop* node) = 0;
      virtual void visit(multop* node) = 0;
      virtual void visit(automatop* node) = 0;
    };

    /// \brief Formula visitor that cannot modify the formula.
    ///
    /// Writing visitors is the prefered way
    /// to traverse a formula, since it doesn't
    /// involve any cast.
    ///
    /// If you want to modify the visited formula, inherit from
    /// spot::eltl:visitor instead.
    struct const_visitor
    {
      virtual ~const_visitor() {}
      virtual void visit(const atomic_prop* node) = 0;
      virtual void visit(const constant* node) = 0;
      virtual void visit(const binop* node) = 0;
      virtual void visit(const unop* node) = 0;
      virtual void visit(const multop* node) = 0;
      virtual void visit(const automatop* node) = 0;
    };

  }
}

#endif /* !SPOT_ELTLAST_FORMULA_HH_ */
