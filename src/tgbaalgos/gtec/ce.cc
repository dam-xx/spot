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
  namespace
  {
    typedef std::pair<const spot::state*, tgba_succ_iterator*> pair_state_iter;
    typedef std::pair<const state*, bdd> state_proposition;
  }

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
    /// FIXME: Should compute label and acceptance condition.
    tgba_run::step s = { spi.first, bddtrue, bddfalse };
    run_->prefix.push_front(s);

    // We build a path trough each SCC in the stack.  For the
    // first SCC, the starting state is the initial state of the
    // automaton.  The destination state is the closest state
    // from the next SCC.  This destination state becomes the
    // starting state when building a path though the next SCC.
    for (int k = 0; k < comp_size - 1; ++k)
      {
	// FIFO for the breadth-first search.
	// (we are looking for the closest state in the next SCC.)
	std::deque<pair_state_iter> todo;

	// Record the father of each state, while performing the BFS.
	typedef std::map<const state*, const state*,
	                 state_ptr_less_than> father_map;
	father_map father;

	// Initial state of the BFS.
	const state* start = run_->prefix.back().s;
	{
	  tgba_succ_iterator* i = ecs_->aut->succ_iter(start);
	  todo.push_back(pair_state_iter(start, i));
	}

	while (!todo.empty())
	  {
	    const state* src = todo.front().first;
	    tgba_succ_iterator* i = todo.front().second;
	    todo.pop_front();

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

			/// FIXME: Should compute label and acceptance
			/// condition.
			tgba_run::step s = { h_dest, bddtrue, bddfalse };
			seq.push_front(s);
			while (src->compare(start))
			  {
			    /// FIXME: Should compute label and acceptance
			    /// condition.
			    tgba_run::step s = { h_dest, bddtrue, bddfalse };
			    seq.push_front(s);
			    src = father[src];
			  }
			// Append SEQ to RUN_->PREFIX.
			run_->prefix.splice(run_->prefix.end(), seq);
			// Exit this BFS for this SCC.
			while (!todo.empty())
			  {
			    delete todo.front().second;
			    todo.pop_front();
			  }
			break;
		      }
		    // Restrict the BFS to state inside the SCC.
		    delete dest;
		    continue;
		  }

		if (father.find(h_dest) == father.end())
		  {
		    todo.push_back
		      (pair_state_iter(h_dest, ecs_->aut->succ_iter(h_dest)));
		    father[h_dest] = src;
		  }
	      }
	    delete i;
	  }
      }

    accepting_path(scc[comp_size - 1], run_->prefix.back().s,
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

  void
  couvreur99_check_result::complete_cycle(const explicit_connected_component*
					  scc,
					  const state* from,
					  const state* to)
  {
    // If by chance our cycle already ends on the state we have
    // to reach back, we are done.
    if (from == to
	&& !run_->cycle.empty())
      return;

    // Records backlinks to parent state during the BFS.
    // (This also stores the propositions of this link.)
    std::map<const state*, state_proposition, state_ptr_less_than> father;

    // BFS queue.
    std::deque<pair_state_iter> todo;

    // Initial state.
    {
      tgba_succ_iterator* i = ecs_->aut->succ_iter(from);
      todo.push_back(pair_state_iter(from, i));
    }

    while (!todo.empty())
      {
	const state* src = todo.front().first;
	tgba_succ_iterator* i = todo.front().second;
	todo.pop_front();
	for (i->first(); !i->done(); i->next())
	  {
	    const state* dest = i->current_state();

	    // Do not escape this SCC or visit a state already visited.
	    const state* h_dest = scc->has_state(dest);
	    if (!h_dest)
	      {
		delete dest;
		continue;
	      }
	    if (father.find(h_dest) != father.end())
	      continue;

	    bdd cond = i->current_condition();

	    // If we have reached our destination, unwind the path
	    // and populate RUN_->CYCLE.
	    if (h_dest == to)
	      {
		tgba_run::steps p;
		// FIXME: should compute acceptance condition.
		tgba_run::step s = { h_dest, cond, bddfalse };
		p.push_front(s);
		while (src != from)
		  {
		    const state_proposition& psi = father[src];
		    // FIXME: should compute acceptance condition.
		    tgba_run::step s = { src, psi.second, bddfalse };
		    p.push_front(s);
		    src = psi.first;
		  }
		run_->cycle.splice(run_->cycle.end(), p);

		// Exit the BFS, but release all iterators first.
		while (!todo.empty())
		  {
		    delete todo.front().second;
		    todo.pop_front();
		  }
		break;
	      }

	    // Common case: record backlinks and continue BFS.
	    todo.push_back(pair_state_iter(h_dest,
					   ecs_->aut->succ_iter(h_dest)));
	    father[h_dest] = state_proposition(src, cond);
	  }
	delete i;
      }
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
  couvreur99_check_result::accepting_path(const explicit_connected_component*
					  scc,
					  const state* start, bdd
					  acc_to_traverse)
  {
    // State seen during the DFS.
    typedef Sgi::hash_set<const state*,
                          state_ptr_hash, state_ptr_equal> set_type;
    set_type seen;
    // DFS stack.
    std::stack<triplet> todo;

    while (acc_to_traverse != bddfalse)
      {
	// Initial state.
	{
	  tgba_succ_iterator* i = ecs_->aut->succ_iter(start);
	  i->first();
	  todo.push(triplet(start, i, bddfalse));
	  seen.insert(start);
	}

	// The path being explored currently.
	tgba_run::steps path;
	// The best path seen so far.
	tgba_run::steps best_path;
	// The acceptance conditions traversed by BEST_PATH.
	bdd best_acc = bddfalse;

	while (!todo.empty())
	  {
	    tgba_succ_iterator* iter = todo.top().iter;
	    const state* s = todo.top().s;

	    // Nothing more to explore, backtrack.
	    if (iter->done())
	      {
		todo.pop();
		delete iter;
		seen.erase(s);
		if (!todo.empty())
		  {
		    assert(!path.empty());
		    path.pop_back();
		  }
		continue;
	      }

	    // We must not escape the current SCC.
	    const state* dest = iter->current_state();
	    const state* h_dest = scc->has_state(dest);
	    if (!h_dest)
	      {
		delete dest;
		iter->next();
		continue;
	      }

	    bdd acc = iter->current_acceptance_conditions() | todo.top().acc;
	    tgba_run::step st = { h_dest, iter->current_condition(),
				 iter->current_acceptance_conditions() };
	    path.push_back(st);

	    // Advance iterator for next step.
	    iter->next();

	    if (seen.find(h_dest) == seen.end())
	      {
		// A new state: continue the DFS.
		tgba_succ_iterator* di = ecs_->aut->succ_iter(h_dest);
		di->first();
		todo.push(triplet(h_dest, di, acc));
		seen.insert(h_dest);
		continue;
	      }

	    // We have completed a full cycle.

	    // If we already have a best path, let see if the current
	    // one is better.
	    if (!best_path.empty())
	      {
		// When comparing the merits of two paths, only the
		// acceptance conditions we are trying the traverse
		// are important.
		bdd acc_restrict = acc & acc_to_traverse;
		bdd best_acc_restrict = best_acc & acc_to_traverse;

		// If the best path and the current one traverse the
		// same acceptance conditions, we keep the shorter
		// path.  Otherwise, we keep the path which has the
		// more acceptance conditions.
		if (best_acc_restrict == acc_restrict)
		  {
		    if (best_path.size() <= path.size())
		      goto backtrack_path;
		  }
		else
		  {
		    // `best_acc_restrict >> acc_restrict' is true
		    // when the set of acceptance conditions of
		    // best_acc_restrict is included in the set of
		    // acceptance conditions of acc_restrict.
		    //
		    // FIXME: It would be better to count the number
		    // of acceptance conditions.
		    if (bddtrue != (best_acc_restrict >> acc_restrict))
		      goto backtrack_path;
		  }
	      }

	    // The current path the best one.
	    best_path = path;
	    best_acc = acc;

	  backtrack_path:
	    // Continue exploration from parent to find better paths.
	    // (Do not pop PATH if ITER is done, because that will be
	    // done at the top of the loop, among other things.)
	    if (!iter->done())
	      path.pop_back();
	  }

	// Append our best path to the run_->cycle.
	for (tgba_run::steps::iterator it = best_path.begin();
	     it != best_path.end(); ++it)
	  run_->cycle.push_back(*it);

	// Prepare to find another path for the remaining acceptance
	// conditions.
	acc_to_traverse -= best_acc;
	start = run_->cycle.back().s;
      }

    // Complete the path so that it goes back to its beginning,
    // forming a cycle.
    complete_cycle(scc, start, run_->prefix.back().s);
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
