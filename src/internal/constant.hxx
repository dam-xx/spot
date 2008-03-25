// Copyright (C) 2003, 2005, 2008 Laboratoire d'Informatique de Paris
// 6 (LIP6), département Systèmes Répartis Coopératifs (SRC),
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

/// \file internal/constant.hxx
/// \brief Generic constants implementation
#ifndef SPOT_INTERNAL_CONSTANT_HXX
# define SPOT_INTERNAL_CONSTANT_HXX

#include "constant.hh"

namespace spot
{
  namespace internal
  {
    template<typename T>
    constant<T>::constant(type val)
      : val_(val)
    {
      switch (val)
	{
	case True:
	  this->dump_ = "constant(1)";
	  this->set_key_();
	  return;
	case False:
	  this->dump_ = "constant(0)";
	  this->set_key_();
	  return;
	}
      // Unreachable code.
      assert(0);
    }

    template<typename T>
    constant<T>::~constant()
    {
    }

    template<typename T>
    void
    constant<T>::accept(visitor& v)
    {
      v.visit(this);
    }

    template<typename T>
    void
    constant<T>::accept(const_visitor& v) const
    {
      v.visit(this);
    }

    template<typename T>
    typename constant<T>::type
    constant<T>::val() const
    {
      return val_;
    }

    template<typename T>
    const char*
    constant<T>::val_name() const
    {
      switch (val_)
	{
	case True:
	  return "1";
	case False:
	  return "0";
	}
      // Unreachable code.
      assert(0);
      return 0;
    }

    template<typename T>
    constant<T>*
    constant<T>::false_instance()
    {
      static constant<T> f(constant<T>::False);
      return &f;
    }

    template<typename T>
    constant<T>*
    constant<T>::true_instance()
    {
      static constant<T> t(constant<T>::True);
      return &t;
    }
  }
}

#endif // SPOT_INTERNAL_CONSTANT_HXX
