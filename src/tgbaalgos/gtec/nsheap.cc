// Copyright (C) 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#include "nsheap.hh"

namespace spot
{
  class numbered_state_heap_hash_map_const_iterator :
    public numbered_state_heap_const_iterator
  {
  public:
    numbered_state_heap_hash_map_const_iterator
      (const numbered_state_heap_hash_map::hash_type& h)
	: numbered_state_heap_const_iterator(), h(h)
    {
    }

    ~numbered_state_heap_hash_map_const_iterator()
    {
    }

    virtual void
    first()
    {
      i = h.begin();
    }

    virtual void
    next()
    {
      ++i;
    }

    virtual bool
    done() const
    {
      return i == h.end();
    }

    virtual const state*
    get_state() const
    {
      return i->first;
    }

    virtual int
    get_index() const
    {
      return i->second;
    }

  private:
    numbered_state_heap_hash_map::hash_type::const_iterator i;
    const numbered_state_heap_hash_map::hash_type& h;
  };

  numbered_state_heap_hash_map::~numbered_state_heap_hash_map()
  {
    // Free keys in H.
    hash_type::iterator i = h.begin();
    while (i != h.end())
      {
	// Advance the iterator before deleting the key.
	const state* s = i->first;
	++i;
	delete s;
      }
  }

  const int*
  numbered_state_heap_hash_map::find(const state* s) const
  {
    hash_type::const_iterator i = h.find(s);
    if (i == h.end())
      return 0;
    return &i->second;
  }

  int*
  numbered_state_heap_hash_map::find(const state* s)
  {
    hash_type::iterator i = h.find(s);
    if (i == h.end())
      return 0;
    return &i->second;
  }

  void
  numbered_state_heap_hash_map::insert(const state* s, int index)
  {
    h[s] = index;
  }

  int
  numbered_state_heap_hash_map::size() const
  {
    return h.size();
  }

  numbered_state_heap_const_iterator*
  numbered_state_heap_hash_map::iterator() const
  {
    return new numbered_state_heap_hash_map_const_iterator(h);
  }

  const state*
  numbered_state_heap_hash_map::filter(const state* s) const
  {
    hash_type::const_iterator i = h.find(s);
    assert(i != h.end());
    if (s != i->first)
      delete s;
    return i->first;
  }
}
