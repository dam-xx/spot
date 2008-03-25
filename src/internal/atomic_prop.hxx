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

/// \file internal/atomic_prop.hxx
/// \brief Generic atomic propositions implementation
#ifndef SPOT_INTERNAL_ATOMIC_PROP_HXX
# define SPOT_INTERNAL_ATOMIC_PROP_HXX

#include "atomic_prop.hh"

namespace spot
{
  namespace internal
  {

    template<typename T>
    atomic_prop<T>::atomic_prop(const std::string& name, environment<T>& env)
      : name_(name), env_(&env)
    {
      this->dump_ = "AP(" + name + ")";
      this->set_key_();
    }

    template<typename T>
    atomic_prop<T>::~atomic_prop()
    {
      // Get this instance out of the instance map.
      pair p(name(), &env());
      typename map::iterator i = instances.find(p);
      assert (i != instances.end());
      instances.erase(i);
    }

    template<typename T>
    void
    atomic_prop<T>::accept(visitor& v)
    {
      v.visit(this);
    }

    template<typename T>
    void
    atomic_prop<T>::accept(const_visitor& v) const
    {
      v.visit(this);
    }

    template<typename T>
    const std::string&
    atomic_prop<T>::name() const
    {
      return name_;
    }

    template<typename T>
    environment<T>&
    atomic_prop<T>::env() const
    {
      return *env_;
    }

    template<typename T>
    typename atomic_prop<T>::map atomic_prop<T>::instances;

    template<typename T>
    atomic_prop<T>*
    atomic_prop<T>::instance(const std::string& name, environment<T>& env)
    {
      pair p(name, &env);
      typename map::iterator i = instances.find(p);
      if (i != instances.end())
	{
	  return static_cast<atomic_prop<T>*>(i->second->ref());
	}
      atomic_prop<T>* ap = new atomic_prop<T>(name, env);
      instances[p] = ap;
      return static_cast<atomic_prop<T>*>(ap->ref());
    }

    template<typename T>
    unsigned
    atomic_prop<T>::instance_count()
    {
      return instances.size();
    }

    template<typename T>
    std::ostream&
    atomic_prop<T>::dump_instances(std::ostream& os)
    {

      for (typename map::iterator i = instances.begin();
	   i != instances.end(); ++i)
      {
	  os << i->second << " = " << i->second->ref_count_()
	     << " * atomic_prop(" << i->first.first << ", "
	     << i->first.second->name() << ")" << std::endl;
	}
      return os;
    }

  }
}

#endif // SPOT_INTERNAL_ATOMICPROP_HXX
