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

#ifndef SPOT_TGBAALGOS_NESTEDDFS_HH
# define SPOT_TGBAALGOS_NESTEDDFS_HH

#include "misc/hash.hh"
#include <list>
#include <utility>
#include <ostream>
#include "tgba/tgbatba.hh"
#include "tgbaalgos/minimalce.hh"

namespace spot
{

  enum search_opt
    {
      magic     = 0,
      nested    = 1,
      my_nested = 2
    };

  class nesteddfs_search: public emptyness_search
  {

  public:

    /// Initialize the Nesteddfs Search algorithm on the automaton \a a.
    nesteddfs_search(const tgba_tba_proxy *a, int opt = nested);

    virtual ~nesteddfs_search();

    /// \brief Perform a Magic or a Nested DFS Search.
    ///
    /// \return true iff the algorithm has found a new accepting
    ///    path.
    ///
    /// check() can be called several times until it return false,
    /// to enumerate all accepting paths.
    virtual ce::counter_example* check();

    /// \brief Print the last accepting path found.
    ///
    /// Restrict printed states to \a the state space of restrict if
    /// supplied.
    std::ostream& print_result(std::ostream& os,
			       const tgba* restrict = 0) const;

    /// \brief Print Stat.
    std::ostream& print_stat(std::ostream& os) const;

  private:

    // The names "stack", "h", and "x", are those used in the paper.

    /// \brief  Records whether a state has be seen with the magic bit
    /// on or off.
    struct magic
    {
      bool seen_without : 1;
      bool seen_with    : 1;
      bool seen_path    : 1;
      //unsigned int depth;
    };

    /// \brief A state for the spot::magic_search algorithm.
    struct magic_state
    {
      const state* s;
      bool m;			///< The state of the magic demon.
      // int depth;
    };

    typedef std::pair<magic_state, tgba_succ_iterator*> state_iter_pair;
    typedef std::list<state_iter_pair> stack_type;
    stack_type stack;		///< Stack of visited states on the path.

    typedef std::list<bdd> tstack_type;
    /// \brief Stack of transitions.
    ///
    /// This is an addition to the data from the paper.
    tstack_type tstack;

    typedef Sgi::hash_map<const state*, magic,
			  state_ptr_hash, state_ptr_equal> hash_type;
    hash_type h;		///< Map of visited states.

    /// Append a new state to the current path.
    bool push(const state* s, bool m);
    /// Check whether we already visited \a s with the Magic bit set to \a m.
    bool has(const state* s, bool m) const;
    /// Check if \a s is in the path.
    bool exist_path(const state* s) const;
    /// Return the depth of the state \a s in stack.
    int depth_path(const state* s) const;

    void build_counter();

    const tgba_tba_proxy* a;	///< The automata to check.
    /// The state for which we are currently seeking an SCC.
    const state* x;
    /// \brief Active the nested search which produce a
    /// smaller counter example.
    bool nested_;
    /// \brief Active the nested bis search which produce a
    /// smaller counter example.
    const state* x_bis;
    bool my_nested_;
    bool accepted_path_;
    int accepted_depth_;

    unsigned int Maxsize;

    ce::counter_example* counter_;

  };


}

#endif // SPOT_TGBAALGOS_NESTEDDFS_HH
