// Copyright (C) 2003, 2005  Laboratoire d'Informatique de Paris 6 (LIP6),
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

/// \file internal/binop.hxx
/// \brief Generic binary operators implementation
#ifndef SPOT_INTERNAL_BINOP_HXX
# define SPOT_INTERNAL_BINOP_HXX

#include "binop.hh"

namespace spot
{
  namespace internal
  {
    template<typename T>
    binop<T>::binop(type op, formula<T>* first, formula<T>* second)
      : op_(op), first_(first), second_(second)
    {
      this->dump_ = "binop(";
      this->dump_ += op_name();
      this->dump_ += ", " + first->dump() + ", " + second->dump() + ")";
      this->set_key_();
    }

    template<typename T>
    binop<T>::~binop()
    {
      // Get this instance out of the instance map.
      pairf pf(first(), second());
      pair p(op(), pf);
      typename map::iterator i = instances.find(p);
      assert (i != instances.end());
      instances.erase(i);
    }

    template<typename T>
    void
    binop<T>::accept(visitor& v)
    {
      v.visit(this);
    }

    template<typename T>
    void
    binop<T>::accept(const_visitor& v) const
    {
      v.visit(this);
    }

    template<typename T>
    const formula<T>*
    binop<T>::first() const
    {
      return first_;
    }

    template<typename T>
    formula<T>*
    binop<T>::first()
    {
      return first_;
    }

    template<typename T>
    const formula<T>*
    binop<T>::second() const
    {
      return second_;
    }

    template<typename T>
    formula<T>*
    binop<T>::second()
    {
      return second_;
    }

    template<typename T>
    typename binop<T>::type
    binop<T>::op() const
    {
      return op_;
    }

    template<typename T>
    const char*
    binop<T>::op_name() const
    {
      return T::binop_name(op_);
    }

    template<typename T>
    typename binop<T>::map binop<T>::instances;

    template<typename T>
    binop<T>*
    binop<T>::instance(type op, formula<T>* first, formula<T>* second)
    {
      // Sort the operands of associative operators, so that for
      // example the formula instance for 'a xor b' is the same as
      // that for 'b xor a'.
      switch (op)
      {
	case T::Xor:
	case T::Equiv:
	  if (second < first)
	    std::swap(first, second);
	  break;
	case T::Implies:
	  // case U:
	  // case R:
	  //   // Non associative operators.
	default:
	  break;
      }

      pairf pf(first, second);
      pair p(op, pf);
      typename map::iterator i = instances.find(p);
      if (i != instances.end())
	{
	  return static_cast<binop<T>*>(i->second->ref());
	}
      binop<T>* ap = new binop<T>(op, first, second);
      instances[p] = ap;
      return static_cast<binop<T>*>(ap->ref());
    }

    template<typename T>
    unsigned
    binop<T>::instance_count()
    {
      return instances.size();
    }
  }
}

#endif // SPOT_INTERNAL_BINOP_HXX
