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

#ifndef SPOT_TGBAALGOS_GTEC_NSHEAP_HH
# define SPOT_TGBAALGOS_GTEC_NSHEAP_HH

#include "tgba/state.hh"
#include "misc/hash.hh"

namespace spot
{
  /// Iterator on numbered_state_heap objects.
  class numbered_state_heap_const_iterator
  {
  public:
    virtual ~numbered_state_heap_const_iterator() {}

    //@{
    /// Iteration
    virtual void first() = 0;
    virtual void next() = 0;
    virtual bool done() const = 0;
    //@}

    //@{
    /// Inspection
    virtual const state* get_state() const = 0;
    virtual int get_index() const = 0;
    //@}
  };

  /// Keep track of a large quantity of indexed states.
  class numbered_state_heap
  {
  public:
    virtual ~numbered_state_heap() {}
    //@{
    /// \brief Is state in the heap?
    ///
    /// Returns 0 if \a s is not in the heap. or a pointer to
    /// its number if it is.
    virtual const int* find(const state* s) const = 0;
    virtual int* find(const state* s) = 0;
    //@}

    /// Add a new state \a s with index \a index
    virtual void insert(const state* s, int index) = 0;

    /// The number of stored states.
    virtual int size() const = 0;

    /// Return an iterator on the states/indexes pairs.
    virtual numbered_state_heap_const_iterator* iterator() const = 0;

    /// \brief Filter state clones.
    ///
    /// Return a state which is equal to \a s, but is an actual key in
    /// the heap, and free \a s if it is a clone of that state.
    ///
    /// Doing so simplify memory management, because we don't have to
    /// track which state need to be kept or deallocated: all key in
    /// the heap should last for the whole life of the emptiness-check.
    virtual const state* filter(const state* s) const = 0;
  };

  /// Abstract factory for numbered_state_heap
  class numbered_state_heap_factory
  {
  public:
    virtual ~numbered_state_heap_factory() {}
    virtual numbered_state_heap* build() const = 0;
  };

  /// A straightforward implementation of numbered_state_heap with a hash map.
  class numbered_state_heap_hash_map : public numbered_state_heap
  {
  public:
    virtual ~numbered_state_heap_hash_map();

    virtual const int* find(const state* s) const;
    virtual int* find(const state* s);
    virtual void insert(const state* s, int index);
    virtual int size() const;

    virtual numbered_state_heap_const_iterator* iterator() const;

    virtual const state* filter(const state* s) const;

  protected:
    typedef Sgi::hash_map<const state*, int,
			  state_ptr_hash, state_ptr_equal> hash_type;
    hash_type h;		///< Map of visited states.

    friend class numbered_state_heap_hash_map_const_iterator;
  };

  /// \brief Factory for numbered_state_heap_hash_map.
  ///
  /// This class is a singleton.  Retrieve the instance using instance().
  class numbered_state_heap_hash_map_factory:
    public numbered_state_heap_factory
  {
  public:
    virtual numbered_state_heap_hash_map* build() const;

    /// Get the unique instance of this class.
    static const numbered_state_heap_hash_map_factory* instance();
  protected:
    virtual ~numbered_state_heap_hash_map_factory() {}
    numbered_state_heap_hash_map_factory();
  };

}

#endif // SPOT_TGBAALGOS_GTEC_NSHEAP_HH
