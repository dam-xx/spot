// Copyright (C) 2010, 2011 Laboratoire de Recherche et Développement de
// l'Epita (LRDE)
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

#include "safety.hh"

namespace spot
{
  bool
  is_safety_automaton(const tgba* aut, const scc_map* sm)
  {
    // Create an scc_map of the user did not give one to us.
    bool need_sm = !sm;
    if (need_sm)
      {
	scc_map* x = new scc_map(aut);
	x->build_map();
	sm = x;
      }

    bool result = true;

    unsigned scc_count = sm->scc_count();
    for (unsigned scc = 0; (scc < scc_count) && result; ++scc)
      {
	if (!sm->accepting(scc))
	  continue;
	// Accepting SCCs should have only one state.
	const std::list<const state*>& st = sm->states_of(scc);
	if (st.size() != 1)
	  {
	    result = false;
	    break;
	  }
	// The state should have only one transition that is a
	// self-loop labelled by true.
	const state* s = *st.begin();
	tgba_succ_iterator* it = aut->succ_iter(s);
	it->first();
	assert(!it->done());
	state* dest = it->current_state();
	bdd cond = it->current_condition();
	it->next();
	result = (!dest->compare(s)) && it->done() && (cond == bddtrue);
	dest->destroy();
	delete it;
      }

    // Free the scc_map if we created it.
    if (need_sm)
      delete sm;

    return result;
  }




}
