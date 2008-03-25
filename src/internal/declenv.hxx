// Copyright (C) 2004, 2008 Laboratoire d'Informatique de Paris 6
// (LIP6), département Systèmes Répartis Coopératifs (SRC), Université
// Pierre et Marie Curie.
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

#ifndef SPOT_INTERNAL_DECLENV_HXX
# define SPOT_INTERNAL_DECLENV_HXX

#include "declenv.hh"

namespace spot
{
  namespace internal
  {

    template<typename T>
    declarative_environment<T>::declarative_environment()
    {
    }

    template<typename T>
    declarative_environment<T>::~declarative_environment()
    {
      // FIXME !!
      // for (prop_map::iterator i = props_.begin(); i != props_.end(); ++i)
      // 	ltl::destroy(i->second);
    }

    template<typename T>
    bool
    declarative_environment<T>::declare(const std::string& prop_str)
    {
      if (props_.find(prop_str) != props_.end())
	return false;
      props_[prop_str] = internal::atomic_prop<T>::instance(prop_str, *this);
      return true;
    }

    template<typename T>
    formula<T>*
    declarative_environment<T>::require(const std::string& prop_str)
    {
      typename prop_map::iterator i = props_.find(prop_str);
      if (i == props_.end())
	return 0;
      // It's an atomic_prop, so we do not have to use the clone() visitor.
      return i->second->ref();
    }

    template<typename T>
    const std::string&
    declarative_environment<T>::name()
    {
      static std::string name("declarative environment");
      return name;
    }

    template<typename T>
    const typename declarative_environment<T>::prop_map&
    declarative_environment<T>::get_prop_map() const
    {
      return props_;
    }
  }
}

#endif // SPOT_INTERNAL_DECLENV_HXX
