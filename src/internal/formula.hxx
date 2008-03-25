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

/// \file internal/formula.hxx
/// \brief Generic formula implementation
#ifndef SPOT_INTERNAL_FORMULA_HXX
# define SPOT_INTERNAL_FORMULA_HXX

#include "formula.hh"

namespace spot
{
  namespace internal
  {
    template<typename T>
    formula<T>*
    formula<T>::ref()
    {
      ref_();
      return this;
    }

    template<typename T>
    formula<T>::~formula()
    {
    }

    template<typename T>
    void
    formula<T>::unref(formula<T>* f)
    {
      if (f->unref_())
	delete f;
    }

    template<typename T>
    void
    formula<T>::ref_()
    {
      // Not reference counted by default.
    }

    template<typename T>
    bool
    formula<T>::unref_()
    {
      // Not reference counted by default.
      return false;
    }

    template<typename T>
    const std::string&
    formula<T>::dump() const
    {
      return dump_;
    }

    template<typename T>
    void
    formula<T>::set_key_()
    {
      string_hash sh;
      hash_key_ = sh(dump_);
    }
  }
}

#endif // SPOT_INTERNAL_FORMULA_HXX
