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

/// \file internal/unop.hxx
/// \brief Generic unary operators implementation
#ifndef SPOT_INTERNAL_UNOP_HXX
# define SPOT_INTERNAL_UNOP_HXX

#include "unop.hh"

namespace spot
{
  namespace internal
  {
    template<typename T>
    unop<T>::unop(type op, formula<T>* child)
      : op_(op), child_(child)
    {
      this->dump_ = "unop(";
      this->dump_ += op_name();
      this->dump_ += ", " + child->dump() + ")";
      this->set_key_();
    }

    template<typename T>
    unop<T>::~unop()
    {
      // Get this instance out of the instance map.
      pair p(op(), child());
      typename map::iterator i = instances.find(p);
      assert (i != instances.end());
      instances.erase(i);
    }

    template<typename T>
    void
    unop<T>::accept(visitor& v)
    {
      v.visit(this);
    }

    template<typename T>
    void
    unop<T>::accept(const_visitor& v) const
    {
      v.visit(this);
    }

    template<typename T>
    const formula<T>*
    unop<T>::child() const
    {
      return child_;
    }

    template<typename T>
    formula<T>*
    unop<T>::child()
    {
      return child_;
    }

    template<typename T>
    typename unop<T>::type
    unop<T>::op() const
    {
      return op_;
    }

    template<typename T>
    const char*
    unop<T>::op_name() const
    {
      return T::unop_name(op_);
    }

    template<typename T>
    typename unop<T>::map unop<T>::instances;

    template<typename T>
    unop<T>*
    unop<T>::instance(type op, formula<T>* child)
    {
      pair p(op, child);
      typename map::iterator i = instances.find(p);
      if (i != instances.end())
	{
	  return static_cast<unop<T>*>(i->second->ref());
	}
      unop<T>* ap = new unop<T>(op, child);
      instances[p] = ap;
      return static_cast<unop<T>*>(ap->ref());
    }

    template<typename T>
    unsigned
    unop<T>::instance_count()
    {
      return instances.size();
    }
  }
}

#endif // SPOT_INTERNAL_UNOP_HXX
