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

#include "ce.hh"
#include "tgba/bddprint.hh"
#include <map>

namespace spot
{
  couvreur99_check_result::couvreur99_check_result
  (const couvreur99_check_status* ecs,
   const explicit_connected_component_factory* eccf)
    : ecs_(ecs), eccf_(eccf)
  {
  }

  tgba_run*
  couvreur99_check_result::accepting_run()
  {
    run_ = new tgba_run;

    assert(!ecs_->root.empty());

    scc_stack::stack_type root = ecs_->root.s;
    int comp_size = root.size();
    // Transform the stack of connected component into an array.
    explicit_connected_component** scc =
      new explicit_connected_component*[comp_size];
    for (int j = comp_size - 1; 0 <= j; --j)
      {
	scc[j] = eccf_->build();
	scc[j]->index = root.top().index;
	scc[j]->condition = root.top().condition;
	root.pop();
      }
    assert(root.empty());

    // Build the set of states for all SCCs.
    numbered_state_heap_const_iterator* i = ecs_->h->iterator();
    for (i->first(); !i->done(); i->next())
      {
	int index = i->get_index();
	// Skip states from dead SCCs.
	if (index < 0)
	  continue;
	assert(index != 0);

	// Find the SCC this state belongs to.
	int j;
	for (j = 1; j < comp_size; ++j)
	  if (index < scc[j]->index)
	    break;
	scc[j - 1]->insert(i->get_state());
      }
    delete i;

    numbered_state_heap::state_index_p spi =
      ecs_->h->index(ecs_->aut->get_init_state());
    assert(spi.first);
    // This incomplete starting step will be overwritten later.
    tgba_run::step s = { spi.first, bddtrue, bddfalse };
    run_->prefix.push_front(s);

    // We build a path trough each SCC in the stack.  For the
    // first SCC, the starting state is the initial state of the
    // automaton.  The destination state is the closest state
    // from the next SCC.  This destination state becomes the
    // starting state when building a path through the next SCC.
    for (int k = 0; k < comp_size - 1; ++k)
      {
	// FIFO for the breadth-first search.
	// (we are looking for the closest state in the next SCC.)
	std::deque<const state*> todo;

	// Record the father of each state, while performing the BFS.
	typedef std::map<const state*, tgba_run::step,
	                 state_ptr_less_than> father_map;
	father_map father;

	// Initial state of the BFS.
	const state* start = run_->prefix.back().s;
	todo.push_back(start);

	while (!todo.empty())
	  {
	    const state* src = todo.front();
	    todo.pop_front();
	    tgba_succ_iterator* i = ecs_->aut->succ_iter(src);

	    for (i->first(); !i->done(); i->next())
	      {
		const state* dest = i->current_state();

		// Are we leaving this SCC?
		const state* h_dest = scc[k]->has_state(dest);
		if (!h_dest)
		  {
		    // If we have found a state in the next SCC.
		    // Unwind the path and populate RUN_->PREFIX.
		    h_dest = scc[k+1]->has_state(dest);
		    if (h_dest)
		      {
			tgba_run::steps seq;

			// The condition and acceptance conditions
			// for the transition leaving H_DEST will
			// be overwritten later when we know them.

			tgba_run::step s = { h_dest, bddtrue, bddfalse };
			seq.push_front(s);

			// Now unwind our track until we reach START.
			tgba_run::step t =
			  { src,
			    i->current_condition(),
			    i->current_acceptance_conditions() };
			while (t.s->compare(start))
			  {
			    seq.push_front(t);
			    t = father[t.s];
			  }
			assert(!run_->prefix.empty());
			// Overwrite the incomplete starting step.
			run_->prefix.back() = t;

			// Append SEQ to RUN_->PREFIX.
			run_->prefix.splice(run_->prefix.end(), seq);

			// Exit this BFS for this SCC.
			todo.clear();
			break;
		      }
		    // Restrict the BFS to state inside the SCC.
		    delete dest;
		    continue;
		  }

		if (father.find(h_dest) == father.end())
		  {
		    todo.push_back(h_dest);
		    tgba_run::step s = { src,
					 i->current_condition(),
					 i->current_acceptance_conditions() };
		    father[h_dest] = s;
		  }
	      }
	    delete i;
	  }
      }

    accepting_cycle(scc[comp_size - 1], run_->prefix.back().s,
		    scc[comp_size - 1]->condition);
    run_->prefix.pop_back(); // this state belongs to the cycle.

    for (int j = comp_size - 1; 0 <= j; --j)
      delete scc[j];
    delete[] scc;

    // Clone every state in the run before returning it.  (We didn't
    // do that before in the algorithm, because it's easier to follow
    // if every state manipulated is the instance in the hash table.)
    for (tgba_run::steps::iterator i = run_->prefix.begin();
	 i != run_->prefix.end(); ++i)
      i->s = i->s->clone();
    for (tgba_run::steps::iterator i = run_->cycle.begin();
	 i != run_->cycle.end(); ++i)
      i->s = i->s->clone();

    return run_;
  }

  namespace
  {
    struct triplet
    {
      const state* s;		// Current state.
      tgba_succ_iterator* iter;	// Iterator to successor of the current state.
      bdd acc;			// All acceptance conditions traversed by
				// the path so far.

      triplet (const state* s, tgba_succ_iterator* iter, bdd acc)
	: s(s), iter(iter), acc(acc)
      {
      }
    };

  }

  void
  couvreur99_check_result::accepting_cycle(const explicit_connected_component*
					   scc,
					   const state* start, bdd
					   acc_to_traverse)
  {
    // Compute an accepting cycle using successive BFS that are
    // restarted from the point reached after we have discovered a
    // transition with a new acceptance conditions.
    //
    // This idea is taken from Product<T>::findWitness in LBTT 1.1.2.
    const state* substart = start;
    do
      {
	// Records backlinks to parent state during the BFS.
	// (This also stores the propositions of this link.)
	std::map<const state*, tgba_run::step, state_ptr_less_than> father;

	// BFS queue.
	std::deque<const state*> todo;

	// Initial state.
	todo.push_back(substart);

	while (!todo.empty())
	  {
	    const state* src = todo.front();
	    todo.pop_front();
	    tgba_succ_iterator* i = ecs_->aut->succ_iter(src);
	    for (i->first(); !i->done(); i->next())
	      {
		const state* dest = i->current_state();

		// Do not escape this SCC
		const state* h_dest = scc->has_state(dest);
		if (!h_dest)
		  {
		    delete dest;
		    continue;
		  }

		bdd cond = i->current_condition();
		bdd acc = i->current_acceptance_conditions();
		tgba_run::step s = { src, cond, acc };

		// If this new step diminish the number of acceptance
		// conditions, record the path so far and start a new
		// BFS for the remaining acceptance conditions.
		//
		// If we have already collected all acceptance conditions,
		// we stop if we cycle back to the start of the cycle.
		bdd less_acc = acc_to_traverse - acc;
		if (less_acc != acc_to_traverse
		    || (acc_to_traverse == bddfalse
			&& h_dest == start))
		  {
		    acc_to_traverse = less_acc;

		    tgba_run::steps p;

		    while (s.s != substart)
		      {
			p.push_front(s);
			s = father[s.s];
		      }
		    p.push_front(s);
		    run_->cycle.splice(run_->cycle.end(), p);

		    // Exit this BFS, and start a new one at h_dest.
		    todo.clear();
		    substart = h_dest;
		    break;
		  }

		// Common case: record backlinks and continue BFS
		// for unvisited states.
		if (father.find(h_dest) == father.end())
		  {
		    todo.push_back(h_dest);
		    father[h_dest] = s;
		  }
	      }
	    delete i;
	  }
      }
    while (acc_to_traverse != bddfalse || substart != start);
  }

  void
  couvreur99_check_result::print_stats(std::ostream& os) const
  {
    ecs_->print_stats(os);
    // FIXME: This is bogusly assuming run_ exists.  (Even if we
    // created it, the user might have delete it.)
    os << run_->prefix.size() << " states in run_->prefix" << std::endl;
    os << run_->cycle.size() << " states in run_->cycle" << std::endl;
  }

}
