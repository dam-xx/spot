// Copyright (C) 2008, 2009  Laboratoire de Recherche et Developpement de
// l'Epita.
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

#ifndef SPOT_TGBAALGOS_SCC_HH
# define SPOT_TGBAALGOS_SCC_HH

#include <map>
#include <stack>
#include <vector>
#include "tgba/tgba.hh"
#include <iosfwd>
#include "misc/hash.hh"
#include "misc/bddlt.hh"

namespace spot
{

  struct scc_stats
  {
    /// Total number of SCCs.
    unsigned scc_total;
    /// Total number of accepting SCC.
    unsigned acc_scc;
    /// Total number of dead SCC.
    ///
    /// An SCC is dead if no accepting SCC is reachable from it.
    /// Note that an SCC can be neither dead nor accepting.
    unsigned dead_scc;

    /// Number of path to a terminal accepting SCC.
    ///
    /// A terminal accepting SCC is an accepting SCC that has
    /// only dead successors (or no successors at all).
    unsigned acc_paths;
    /// Number of paths to a terminal dead SCC.
    ///
    /// A terminal dead SCC is a dead SCC without successors.
    unsigned dead_paths;

    std::ostream& dump(std::ostream& out) const;
  };

  class scc_map
  {
  public:
    typedef std::map<unsigned, bdd> succ_type;
    typedef std::set<bdd, bdd_less_than> cond_set;

    scc_map(const tgba* aut);

    void build_map();

    const tgba* get_aut() const;

    unsigned scc_count() const;

    unsigned initial() const;

    const succ_type& succ(unsigned n) const;
    bool accepting(unsigned n) const;
    const cond_set& cond_set_of(unsigned n) const;
    bdd acc_set_of(unsigned n) const;
    const std::list<const state*>& states_of(unsigned n) const;

    unsigned scc_of_state(const state* s) const;

  protected:

    int relabel_component();

    struct scc
    {
    public:
      scc(int index) : index(index), acc(bddfalse) {};
      /// Index of the SCC.
      int index;
      /// The union of all acceptance conditions of transitions which
      /// connect the states of the connected component.
      bdd acc;
      /// States of the component.
      std::list<const state*> states;
      /// Set of conditions used in the SCC.
      cond_set conds;
      /// Successor SCC.
      succ_type succ;
    };

    const tgba* aut_;		// Automata to decompose.
    typedef std::list<scc> stack_type;
    stack_type root_;		// Stack of SCC roots.
    std::stack<bdd> arc_acc_;	// A stack of acceptance conditions
				// between each of these SCC.
    std::stack<bdd> arc_cond_;	// A stack of conditions
				// between each of these SCC.
    typedef Sgi::hash_map<const state*, int,
			  state_ptr_hash, state_ptr_equal> hash_type;
    hash_type h_;		// Map of visited states.  Values >= 0
                                // designate maximal SCC.  Values < 0
                                // number states that are part of
                                // incomplete SCCs being completed.
    int num_;			// Number of visited nodes, negated.
    typedef std::pair<const spot::state*, tgba_succ_iterator*> pair_state_iter;
    std::stack<pair_state_iter> todo_; // DFS stack.  Holds (STATE,
				       // ITERATOR) pairs where
				       // ITERATOR is an iterator over
				       // the successors of STATE.
				       // ITERATOR should always be
				       // freed when TODO is popped,
				       // but STATE should not because
				       // it is used as a key in H.

    typedef std::vector<scc> scc_map_type;
    scc_map_type scc_map_; // Map of constructed maximal SCC.
			   // SCC number "n" in H_ corresponds to entry
                           // "n" in SCC_MAP_.
  };

  scc_stats build_scc_stats(const tgba* a);
  scc_stats build_scc_stats(const scc_map& m);

  std::ostream& dump_scc_dot(const tgba* a, std::ostream& out,
			     bool verbose = false);
  std::ostream& dump_scc_dot(const scc_map& m, std::ostream& out,
			     bool verbose = false);
}

#endif // SPOT_TGBAALGOS_SCC_HH
