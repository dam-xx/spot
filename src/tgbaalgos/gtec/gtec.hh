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

#ifndef SPOT_TGBAALGOS_GTEC_GTEC_HH
# define SPOT_TGBAALGOS_GTEC_GTEC_HH

#include "status.hh"
#include "tgbaalgos/emptiness.hh"

namespace spot
{
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
  /// it return false, a stack of SCC has been built is available
  /// using result() (spot::counter_example needs it).
  ///
  /// There are two variants of this algorithm: spot::couvreur99_check and
  /// spot::couvreur99_check_shy.  They differ in their memory usage, the
  /// number for successors computed before they are used and the way
  /// the depth first search is directed.
  ///
  /// spot::couvreur99_check performs a straightforward depth first search.
  /// The DFS stacks store tgba_succ_iterators, so that only the
  /// iterators which really are explored are computed.
  ///
  /// spot::couvreur99_check_shy tries to explore successors which are
  /// visited states first.  this helps to merge SCCs and generally
  /// helps to produce shorter counter-examples.  However this
  /// algorithm cannot stores unprocessed successors as
  /// tgba_succ_iterators: it must compute all successors of a state
  /// at once in order to decide which to explore first, and must keep
  /// a list of all unexplored successors in its DFS stack.
  class couvreur99_check: public emptiness_check
  {
  public:
    couvreur99_check(const tgba* a,
		     const numbered_state_heap_factory* nshf
		     = numbered_state_heap_hash_map_factory::instance());
    virtual ~couvreur99_check();

    /// Check whether the automaton's language is empty.
    virtual emptiness_check_result* check();

    /// \brief Return the status of the emptiness-check.
    ///
    /// When check() succeed, the status should be passed along
    /// to spot::counter_example.
    ///
    /// This status should not be deleted, it is a pointer
    /// to a member of this class that will be deleted when
    /// the couvreur99 object is deleted.
    const couvreur99_check_status* result() const;

  protected:
    couvreur99_check_status* ecs_;
    /// \brief Remove a strongly component from the hash.
    ///
    /// This function remove all accessible state from a given
    /// state. In other words, it removes the strongly connected
    /// component that contains this state.
    void remove_component(const state* start_delete);
  };

  /// \brief A version of spot::couvreur99_check that tries to visit
  /// known states first.
  ///
  /// See the documentation for spot::couvreur99_check
  class couvreur99_check_shy : public couvreur99_check
  {
  public:
    couvreur99_check_shy(const tgba* a,
			 const numbered_state_heap_factory* nshf
			 = numbered_state_heap_hash_map_factory::instance());
    virtual ~couvreur99_check_shy();

    virtual emptiness_check_result* check();

  protected:
    struct successor {
      bdd acc;
      const spot::state* s;
      successor(bdd acc, const spot::state* s): acc(acc), s(s) {}
    };

    // We use five main data in this algorithm:
    // * couvreur99_check::root, a stack of strongly connected components (SCC),
    // * couvreur99_check::h, a hash of all visited nodes, with their order,
    //   (it is called "Hash" in Couvreur's paper)
    // * arc, a stack of acceptance conditions between each of these SCC,
    std::stack<bdd> arc;
    // * num, the number of visited nodes.  Used to set the order of each
    //   visited node,
    int num;
    // * todo, the depth-first search stack.  This holds pairs of the
    //   form (STATE, SUCCESSORS) where SUCCESSORS is a list of
    //   (ACCEPTANCE_CONDITIONS, STATE) pairs.
    typedef std::list<successor> succ_queue;
    typedef std::pair<const state*, succ_queue> pair_state_successors;
    std::stack<pair_state_successors> todo;

    /// \brief find the SCC number of a unprocessed state.
    ///
    /// Sometimes we want to modify some of the above structures when
    /// looking up a new state.  This happens for instance when find()
    /// must perform inclusion checking and add new states to process
    /// to TODO during this step.  (Because TODO must be known,
    /// sub-classing spot::numbered_state_heap is not enough.)  Then
    /// overriding this method is the way to go.
    virtual int* find_state(const state* s);
  };

}

#endif // SPOT_TGBAALGOS_GTEC_GTEC_HH
