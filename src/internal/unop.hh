// Copyright (C) 2003, 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
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

/// \file internal/unop.hh
/// \brief Generic unary operators
#ifndef SPOT_INTERNAL_UNOP_HH
# define SPOT_INTERNAL_UNOP_HH

#include <map>
#include "refformula.hh"
#include <cassert>

namespace spot
{
  namespace internal
  {

    /// \brief Generic unary operators.
    /// \ingroup generic_ast
    template<typename T>
    class unop : public ref_formula<T>
    {
    public:
      typedef typename T::unop type;

      /// Build an unary operator with operation \a op and
      /// child \a child.
      static unop* instance(type op, formula<T>* child);

      typedef typename T::visitor visitor;
      typedef typename T::const_visitor const_visitor;

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      /// Get the sole operand of this operator.
      const formula<T>* child() const;
      /// Get the sole operand of this operator.
      formula<T>* child();

      /// Get the type of this operator.
      type op() const;
      /// Get the type of this operator, as a string.
      const char* op_name() const;

      /// Number of instantiated unary operators.  For debugging.
      static unsigned instance_count();

    protected:
      typedef std::pair<type, formula<T>*> pair;
      typedef std::map<pair, formula<T>*> map;
      static map instances;

      unop(type op, formula<T>* child);
      virtual ~unop();

    private:
      type op_;
      formula<T>* child_;
    };

  }
}

# include "unop.hxx"

#endif // SPOT_INTERNAL_UNOP_HH
