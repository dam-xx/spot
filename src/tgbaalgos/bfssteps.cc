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

#include <map>
#include <deque>
#include "bfssteps.hh"
#include "tgba/tgba.hh"

namespace spot
{

  bfs_steps::bfs_steps(const tgba* a)
    : a_(a)
  {
  }

  bfs_steps::~bfs_steps()
  {
  }

  const state*
  bfs_steps::search(const state* start, tgba_run::steps& l)
  {
    // Records backlinks to parent state during the BFS.
    // (This also stores the propositions of this link.)
    std::map<const state*, tgba_run::step,
      state_ptr_less_than> father;
    // BFS queue.
    std::deque<const state*> todo;
    // Initial state.
    todo.push_back(start);

    while (!todo.empty())
      {
	const state* src = todo.front();
	todo.pop_front();
	tgba_succ_iterator* i = a_->succ_iter(src);
	for (i->first(); !i->done(); i->next())
	  {
	    const state* dest = filter(i->current_state());

	    if (!dest)
	      continue;

	    bdd cond = i->current_condition();
	    bdd acc = i->current_acceptance_conditions();
	    tgba_run::step s = { src, cond, acc };

	    if (match(s, dest))
	      {
		// Found it!

		tgba_run::steps p;
		for (;;)
		  {
		    tgba_run::step tmp = s;
		    tmp.s = tmp.s->clone();
		    p.push_front(tmp);
		    if (s.s == start)
		      break;
		    s = father[s.s];
		  }

		l.splice(l.end(), p);
		delete i;
		return dest;
	      }

	    // Common case: record backlinks and continue BFS
	    // for unvisited states.
	    if (father.find(dest) == father.end())
	      {
		todo.push_back(dest);
		father[dest] = s;
	      }
	  }
	delete i;
      }
    return 0;
  }

}
