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

#ifndef SPOT_TGBAALGOS_COLORDFS_HH
# define SPOT_TGBAALGOS_COLORDFS_HH

#include "misc/hash.hh"
#include <list>
#include <utility>
#include <ostream>
#include "tgba/tgbatba.hh"
#include "tgbaalgos/minimalce.hh"

namespace spot
{
  class colordfs_search: public emptyness_search
  {
  public:
    /// Initialize the Colordfs Search algorithm on the automaton \a a.
    colordfs_search(const tgba_tba_proxy *a);

    virtual ~colordfs_search();

    /// \brief Perform a Color DFS Search.
    ///
    /// \return a new accepting path if there exists one, NULL otherwise.
    ///
    /// check() can be called several times until it return false,
    /// to enumerate all accepting paths.
    virtual ce::counter_example* check();

    /// \brief Print Stat.
    std::ostream& print_stat(std::ostream& os) const;

  private:

    // The names "stack", "h", and "x", are those used in the paper.

    /// \brief  Records the color of a state.
    enum color
      {
	white = 0,
	blue  = 1,
	red   = 2,
	black = 3
      };

    struct color_state
    {
      color c;
      bool is_in_cp;
      int depth;
    };

    typedef std::pair<const state*, tgba_succ_iterator*> state_iter_pair;
    typedef std::list<state_iter_pair> stack_type;
    stack_type stack;		///< Stack of visited states on the path.

    typedef std::list<bdd> tstack_type;
    /// \brief Stack of transitions.
    ///
    /// This is an addition to the data from the paper.
    tstack_type tstack;

    typedef Sgi::hash_map<const state*, color_state,
			  state_ptr_hash, state_ptr_equal> hash_type;
    hash_type h;		///< Map of visited states.

    /// The three dfs as explain in
    /// @InProceedings(GaMoZe04spin,
    /// Author = "Gastin, P. and Moro, P. and Zeitoun, M.",
    /// Title = "Minimization of counterexamples in {SPIN}",
    /// BookTitle = "Proceedings of the 11th SPIN Workshop (SPIN'04)",
    /// Editor = "Graf, S. and Mounier, L.",
    /// Publisher = SPRINGER,
    /// Series = LNCS,
    /// Number = 2989,
    /// Year = 2004,
    /// Pages = "92-108")
    bool dfs_blue(const state* s, bdd acc = bddfalse);
    bool dfs_red(const state* s);
    void dfs_black(const state* s);

    /// Append a new state to the current path.
    bool push(const state* s, color c);
    /// Remove a state to the current path.
    void pop();
    /// Check if all successors of \a s are black and color
    /// \a s in black if true.
    bool all_succ_black(const state* s);

    const tgba_tba_proxy* a;	///< The automata to check.
    /// The state for which we are currently seeking an SCC.
    const state* x;

    ce::counter_example* counter_;
    clock_t tps_;
  };


}

#endif // SPOT_TGBAALGOS_COLORDFS_HH
