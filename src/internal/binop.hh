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

/// \file internal/binop.hh
/// \brief Generic binary operators
///
/// This does not include \c AND and \c OR operators.  These are
/// considered to be multi-operand operators (see spot::internal::multop).
#ifndef SPOT_INTERNAL_BINOP_HH
# define SPOT_INTERNAL_BINOP_HH

#include <map>
#include "refformula.hh"
#include <cassert>
#include <utility>

namespace spot
{
  namespace internal
  {

    /// \brief Generic binary operator.
    /// \ingroup generic_ast
    template<typename T>
    class binop : public ref_formula<T>
    {
    public:
      /// Different kinds of binary opertaors
      ///
      /// And and Or are not here.  Because they
      /// are often nested we represent them as multops.
      typedef typename T::binop type;

      /// Build an unary operator with operation \a op and
      /// children \a first and \a second.
      static binop<T>* instance(type op, formula<T>* first, formula<T>* second);

      typedef typename T::visitor visitor;
      typedef typename T::const_visitor const_visitor;

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      /// Get the first operand.
      const formula<T>* first() const;
      /// Get the first operand.
      formula<T>* first();
      /// Get the second operand.
      const formula<T>* second() const;
      /// Get the second operand.
      formula<T>* second();

      /// Get the type of this operator.
      type op() const;
      /// Get the type of this operator, as a string.
      const char* op_name() const;

      /// Number of instantiated binary operators.  For debugging.
      static unsigned instance_count();

    protected:
      typedef std::pair<formula<T>*, formula<T>*> pairf;
      typedef std::pair<type, pairf> pair;
      typedef std::map<pair, formula<T>*> map;
      static map instances;

      binop(type op, formula<T>* first, formula<T>* second);
      virtual ~binop();

    private:
      type op_;
      formula<T>* first_;
      formula<T>* second_;
    };

  }
}

# include "binop.hxx"

#endif // SPOT_INTERNAL_BINOP_HH
