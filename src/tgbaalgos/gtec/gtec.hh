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
  /// There are two variants of this algorithm: spot::emptiness_check and
  /// spot::emptiness_check_shy.  They differ in their memory usage, the
  /// number for successors computed before they are used and the way
  /// the depth first search is directed.
  ///
  /// spot::emptiness_check performs a straightforward depth first search.
  /// The DFS stacks store tgba_succ_iterators, so that only the
  /// iterators which really are explored are computed.
  ///
  /// spot::emptiness_check_shy try to explore successors which are
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

    /// Check whether the automaton's language is empty.
    virtual bool check();

    /// \brief Return the status of the emptiness-check.
    ///
    /// When check() succeed, the status should be passed along
    /// to spot::counter_example.
    ///
    /// This status should not be deleted, it is a pointer
    /// to a member of this class that will be deleted when
    /// the emptiness_check object is deleted.
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

  /// \brief A version of spot::emptiness_check try to visit known
  /// states first.
  ///
  /// See the documentation for spot::emptiness_check
  class emptiness_check_shy : public emptiness_check
  {
  public:
    emptiness_check_shy(const tgba* a);
    virtual ~emptiness_check_shy();

    virtual bool check();
  };

}

#endif // SPOT_TGBAALGOS_GTEC_GTEC_HH
