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

#ifndef SPOT_TGBAALGOS_TARJAN_ON_FLY_HH
# define SPOT_TGBAALGOS_TARJAN_ON_FLY_HH

#include "misc/hash.hh"
#include <list>
#include <utility>
#include <ostream>
#include "tgba/tgbatba.hh"
//#include "tgba/bddprint.hh"
#include "tgbaalgos/minimalce.hh"

namespace spot
{

  class tarjan_on_fly: public emptyness_search
  {

  public:

    tarjan_on_fly(const tgba_tba_proxy *a);
    virtual ~tarjan_on_fly();

    /// \brief Find a counter example.
    virtual ce::counter_example* check();

    /// \brief Print Stat.
    std::ostream& print_stat(std::ostream& os) const;

  private:

    struct struct_state
    {
      const state* s;
      tgba_succ_iterator* lasttr;
      int lowlink;
      int pre;
      int acc;

      //int pos;
    };

    //typedef std::pair<int, tgba_succ_iterator*> state_iter_pair;
    typedef Sgi::hash_map<const state*, int,
			  state_ptr_hash, state_ptr_equal> hash_type;
    hash_type h;		///< Map of visited states.


    //typedef std::pair<const state*, struct_state> pair_type;
    typedef std::vector<struct_state> stack_type;
    stack_type stack;		///< Stack of visited states on the path.

    const tgba_tba_proxy* a;	///< The automata to check.

    int top;
    int dftop;
    bool violation;

    const state* x;
    ce::counter_example* ce;

    void push(const state* s);
    void pop();
    void lowlinkupdate(int f, int t);
    int in_stack(const state* s) const;

    ce::counter_example* build_counter();
    //clock_t tps_;

  };

}

#endif // SPOT_TGBAALGOS_MINIMALCE_HH
