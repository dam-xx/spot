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

#ifndef SPOT_EMPTINESS_CHECK_HH
# define SPOT_EMPTINESS_CHECK_HH

#include "tgba/tgba.hh"
#include "misc/hash.hh"
#include <stack>
#include <list>
#include <utility>
#include <iostream>

namespace spot
{

  class scc_stack
  {
  public:
    struct connected_component
    {
      // During the Depth path we keep the connected component that we met.
    public:
      connected_component(int index = -1);

      int index;
      /// The bdd condition is the union of all acceptance conditions of
      /// transitions which connect the states of the connected component.
      bdd condition;
    };

    connected_component& top();
    void pop();
    void push(int index);
    size_t size() const;
    bool empty() const;

    typedef std::stack<connected_component> stack_type;
    stack_type s;
  };

  class numbered_state_heap_const_iterator
  {
  public:
    virtual ~numbered_state_heap_const_iterator() {}

    virtual void first() = 0;
    virtual void next() = 0;
    virtual bool done() const = 0;

    virtual const state* get_state() const = 0;
    virtual int get_index() const = 0;
  };

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

    virtual void insert(const state* s, int index) = 0;
    virtual int size() const = 0;

    virtual numbered_state_heap_const_iterator* iterator() const = 0;

    /// \brief Return a state which is equal to \a s, but is in \c h,
    /// and free \a s if it is different.
    ///
    /// Doing so simplify memory management, because we don't have to
    /// track which state need to be kept or deallocated: all key in
    /// \c h should last for the whole life of the emptiness_check.
    virtual const state* filter(const state* s) const = 0;
  };

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

  class emptiness_check_status
  {
  public:
    emptiness_check_status(const tgba* aut);
    ~emptiness_check_status();

    const tgba* aut;
    scc_stack root;
    numbered_state_heap_hash_map h; ///< Map of visited states.

    /// Output statistics about this object.
    void print_stats(std::ostream& os) const;
  };

  //@{
  /// \brief Check whether the language of an automate is empty.
  ///
  /// This is based on the following paper.
  /// \verbatim
  /// @InProceedings{couvreur.99.fm,
  ///   author    = {Jean-Michel Couvreur},
  ///   title     = {On-the-fly Verification of Temporal Logic},
  ///   pages     = {253--271},
  ///   editor    = {Jeannette M. Wing and Jim Woodcock and Jim Davies},
  ///   booktitle = {Proceedings of the World Congress on Formal Methods in
  ///                the Development of Computing Systems (FM'99)},
  ///   publisher = {Springer-Verlag},
  ///   series    = {Lecture Notes in Computer Science},
  ///   volume    = {1708},
  ///   year      = {1999},
  ///   address   = {Toulouse, France},
  ///   month     = {September},
  ///   isbn      = {3-540-66587-0}
  /// }
  /// \endverbatim
  ///
  /// check() returns true if the automaton's language is empty.  When
  /// it return false, a stack of SCC has been built and can
  /// later be used by counter_example().
  ///
  /// There are two variants of this algorithm: emptiness_check() and
  /// emptiness_check_shy().  They differ in their memory usage, the
  /// number for successors computed before they are used and the way
  /// the depth first search is directed.
  ///
  /// emptiness_check() performs a straightforward depth first search.
  /// The DFS stacks store tgba_succ_iterators, so that only the
  /// iterators which really are explored are computed.
  ///
  /// emptiness_check_shy() try to explore successors which are
  /// visited states first.  this helps to merge SCCs and generally
  /// helps to produce shorter counter-examples.  However this
  /// algorithm cannot stores unprocessed successors as
  /// tgba_succ_iterators: it must compute all successors of a state
  /// at once in order to decide which to explore first, and must keep
  /// a list of all unexplored successors in its DFS stack.
  class emptiness_check
  {
  public:
    emptiness_check(const tgba* a);
    virtual ~emptiness_check();

    /// check whether the automaton's language is empty
    virtual bool check();

    const emptiness_check_status* result() const;

  protected:
    emptiness_check_status* ecs_;
    /// \brief Remove a strongly component from the hash.
    ///
    /// This function remove all accessible state from a given
    /// state. In other words, it removes the strongly connected
    /// component that contains this state.
    void remove_component(const state* start_delete);
  };

  class emptiness_check_shy : public emptiness_check
  {
  public:
    emptiness_check_shy(const tgba* a);
    virtual ~emptiness_check_shy();

    virtual bool check();
  };
  //@}


  class explicit_connected_component: public scc_stack::connected_component
  {
  public:
    virtual ~explicit_connected_component() {}
    /// \brief Check if the SCC contains states \a s.
    ///
    /// Return the representative of \a s in the SCC, and delete \a
    /// s if it is different (acting like
    /// numbered_state_heap::filter), or 0 otherwise.
    virtual const state* has_state(const state* s) const = 0;

    /// Insert a new state in the SCC.
    virtual void insert(const state* s) = 0;
  };

  class connected_component_hash_set: public explicit_connected_component
  {
  public:
    virtual ~connected_component_hash_set() {}
    virtual const state* has_state(const state* s) const;
    virtual void insert(const state* s);
  protected:
    typedef Sgi::hash_set<const state*,
			  state_ptr_hash, state_ptr_equal> set_type;
    set_type states;
  };

  class explicit_connected_component_factory
  {
  public:
    virtual ~explicit_connected_component_factory() {}
    virtual explicit_connected_component* build() const = 0;
  };

  class connected_component_hash_set_factory :
    public explicit_connected_component_factory
  {
  public:
    virtual connected_component_hash_set* build() const;

    static const connected_component_hash_set_factory* instance();

  protected:
    virtual ~connected_component_hash_set_factory() {}
    connected_component_hash_set_factory();
  };

  class counter_example
  {
  public:
    counter_example(const emptiness_check_status* ecs,
		    const explicit_connected_component_factory*
		    eccf = connected_component_hash_set_factory::instance());

    typedef std::pair<const state*, bdd> state_proposition;
    typedef std::list<const state*> state_sequence;
    typedef std::list<state_proposition> cycle_path;
    state_sequence suffix;
    cycle_path period;

    /// \brief Display the example computed by counter_example().
    ///
    /// \param restrict optional automaton to project the example on.
    std::ostream& print_result(std::ostream& os,
			       const tgba* restrict = 0) const;

    /// Output statistics about this object.
    void print_stats(std::ostream& os) const;

  protected:
    /// Called by counter_example to find a path which traverses all
    /// acceptance conditions in the accepted SCC.
    void accepting_path (const explicit_connected_component* scc,
			 const state* start, bdd acc_to_traverse);

    /// Complete a cycle that caraterise the period of the counter
    /// example.  Append a sequence to the path given by accepting_path.
    void complete_cycle(const explicit_connected_component* scc,
			const state* from, const state* to);

  private:
    const emptiness_check_status* ecs_;
  };
}
#endif // SPOT_EMPTINESS_CHECK_HH
