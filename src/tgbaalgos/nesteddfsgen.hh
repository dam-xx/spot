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

#ifndef SPOT_TGBAALGOS_NESTEDDFSGEN_HH
# define SPOT_TGBAALGOS_NESTEDDFSGEN_HH

#include "misc/hash.hh"
#include <list>
#include <utility>
#include <ostream>
#include "tgba/tgba.hh"
#include "tgbaalgos/minimalce.hh"

namespace spot
{
  class nesteddfsgen_search
  {

  public:

    /// Initialize the Nesteddfs Search algorithm on the automaton \a a.
    nesteddfsgen_search(const tgba *a);
    ~nesteddfsgen_search();

    /// \brief Find a counter example.
    //ce::counter_example* check();
    bool check();

    void print_stats(std::ostream& os) const;

  private:

    struct state_info
    {
      bool in_stack  : 1;
      bool processed : 1;
      bdd cond;
    };

    typedef Sgi::hash_map<const state*, state_info,
			  state_ptr_hash, state_ptr_equal> hash_type;
    hash_type h;		///< Map of visited states.
    //hash_type processed;	///< Map of processed states.


    typedef std::pair<const state*, tgba_succ_iterator*> state_iter_pair;
    typedef std::list<state_iter_pair> stack_type;
    stack_type stack;		///< Stack of visited states on the path.

    std::list<const state*> *states;

    void markConditions(const state* s, bdd cond);
    void free_states();

    const tgba* a;
    ce::counter_example* counter_;

  };


}

#endif // SPOT_TGBAALGOS_NESTEDDFS_HH
