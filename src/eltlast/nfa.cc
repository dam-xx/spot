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

#include <cassert>
#include "nfa.hh"

namespace spot
{
  namespace eltl
  {
    nfa::nfa()
      : ns_(), sn_(), init_(0), arity_(0), finals_()
    {
    }

    nfa::~nfa()
    {
      ns_map::iterator i;
      for (i = ns_.begin(); i != ns_.end(); ++i)
      {
	state::iterator i2;
	for (i2 = i->second->begin(); i2 != i->second->end(); ++i2)
	  delete *i2;
	delete i->second;
      }
    }

    nfa::state*
    nfa::add_state(const std::string& name)
    {
      ns_map::iterator i = ns_.find(name);
      if (i == ns_.end())
      {
	state* s = new nfa::state;
	ns_[name] = s;
	sn_[s] = name;

	if (!init_)
	  init_ = s;
	return s;
      }
      return i->second;
    }

    void
    nfa::add_transition(const std::string& s, const std::string& d, unsigned c)
    {
      state* source = add_state(s);
      nfa::transition* t = new transition;
      t->dest = add_state(d);
      t->cost = c;
      source->push_back(t);
      if (c > arity_)
	arity_ = c;
    }

    void
    nfa::set_init_state(const std::string& state)
    {
      init_ = add_state(state);
    }

    void
    nfa::set_final(const std::string& state)
    {
      finals_.insert(state);
    }

    bool
    nfa::is_final(const std::string& state)
    {
      return finals_.find(state) != finals_.end();
    }

    unsigned
    nfa::arity()
    {
      return arity_ + 1;
    }

    const nfa::state*
    nfa::get_init_state()
    {
      if (!init_)
	add_state("empty");
      return init_;
    }

    nfa::iterator
    nfa::begin(const state* s) const
    {
      return nfa::iterator(s->begin());
    }

    nfa::iterator
    nfa::end(const state* s) const
    {
      return nfa::iterator(s->end());
    }

    std::string
    nfa::format_state(const state* s) const
    {
      sn_map::const_iterator i = sn_.find(s);
      assert(i != sn_.end());
      return i->second;
    }
  }
}
