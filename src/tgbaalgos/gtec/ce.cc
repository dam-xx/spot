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
  }

  counter_example::counter_example(const emptiness_check_status* ecs,
				   const explicit_connected_component_factory*
				   eccf)
    : ecs_(ecs)
  {

    std::cout << "counter_example" << std::endl;

    //counter_ = new ce::counter_example(ecs->aut);

    assert(!ecs_->root.empty());
    assert(suffix.empty());

    scc_stack::stack_type root = ecs_->root.s;
    int comp_size = root.size();
    // Transform the stack of connected component into an array.
    explicit_connected_component** scc =
      new explicit_connected_component*[comp_size];
    for (int j = comp_size - 1; 0 <= j; --j)
      {
	scc[j] = eccf->build();
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
    suffix.push_front(spi.first);

    /////
    // counter_->prefix.push_front(ce::state_ce(spi.first->clone(), bddfalse));
    ////

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
	const state* start = suffix.back();
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
		    // If we have found a state in greater SCC which.
		    // Unwind the path and populate SUFFIX.
		    h_dest = scc[k+1]->has_state(dest);
		    if (h_dest)
		      {
			state_sequence seq;

			///
			// ce::l_state_ce seq_count;
			///

			seq.push_front(h_dest);
			while (src->compare(start))
			  {
			    ///
			    // seq_count.push_front(ce::state_ce(src->clone(),
			    // bddfalse));
			    ///

			    seq.push_front(src);
			    src = father[src];
			  }
			// Append SEQ to SUFFIX.
			suffix.splice(suffix.end(), seq);

			///
			// counter_->prefix.splice(counter_->prefix.end(),
			// seq_count);
			///

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

    accepting_path(scc[comp_size - 1], suffix.back(),
		   scc[comp_size - 1]->condition);


    for (int j = comp_size - 1; 0 <= j; --j)
      delete scc[j];
    delete[] scc;
  }

  void
  counter_example::complete_cycle(const explicit_connected_component* scc,
				  const state* from,
				  const state* to)
  {
    //std::cout << "complete_cycle" << std::endl;

    // If by change or period already ends on the state we have
    // to reach back, we are done.
    if (from == to
	&& !period.empty())
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
	    // and populate PERIOD.
	    if (h_dest == to)
	      {
		cycle_path p;

		///
		// ce::l_state_ce p_counter;
		// p_counter.push_front(ce::state_ce(h_dest->clone(), cond));
		///

		p.push_front(state_proposition(h_dest, cond));
		while (src != from)
		  {
		    const state_proposition& psi = father[src];
		    ///
		    // p_counter.push_front(ce::state_ce(src->clone(),
		    // psi.second));
		    ///
		    p.push_front(state_proposition(src, psi.second));
		    src = psi.first;
		  }
		period.splice(period.end(), p);
		///
		// counter_->cycle.splice(counter_->cycle.end(),
		// p_counter);
		///

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

  ////////////////////////////////////////////////////////////////////////
  /*

  void
  counter_example::accepting_path(const explicit_connected_component* scc,
				  const state* start, bdd acc_to_traverse)
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
	cycle_path path;
	// The best path seen so far.
	cycle_path best_path;
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
		if (todo.size())
		  {
		    assert(path.size());
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
	    path.push_back(state_proposition(h_dest,
					     iter->current_condition()));

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
	    if (best_path.size())
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

	// Append our best path to the period.
	for (cycle_path::iterator it = best_path.begin();
	     it != best_path.end(); ++it)
	  {
	    period.push_back(*it);
	    ce::state_ce ce(it->first, it->second);
	    counter_->cycle.push_back(ce);
	    counter_->cycle.push_back(*it);
	  }

	// Prepare to find another path for the remaining acceptance
	// conditions.
	acc_to_traverse -= best_acc;
	start = period.back().first;
      }

    // Complete the path so that it goes back to its beginning,
    // forming a cycle.
    complete_cycle(scc, start, suffix.back());
  }

  */
  ////////////////////////////////////////////////////////////////////////

  void
  counter_example::accepting_path(const explicit_connected_component* scc,
				  const state* start, bdd acc_to_traverse)
  {
    //std::cout << "accepting_path" << std::endl;

    // State seen during the DFS.
    typedef Sgi::hash_set<const state*,
                          state_ptr_hash, state_ptr_equal> set_type;
    set_type seen;
    // DFS stack.
    std::stack<triplet> todo;

    while (acc_to_traverse != bddfalse)
      {
	//std::cout << "accepting_path : while (acc_to_traverse != bddfalse)"
	// << std::endl;

	// Initial state.
	{
	  tgba_succ_iterator* i = ecs_->aut->succ_iter(start);
	  i->first();
	  todo.push(triplet(start, i, bddfalse));
	  seen.insert(start);
	}

	// The path being explored currently.
	cycle_path path;
	// The best path seen so far.
	cycle_path best_path;
	// The acceptance conditions traversed by BEST_PATH.
	bdd best_acc = bddfalse;

	while (!todo.empty())
	  {
	    // std::cout << "accepting_path : while (!todo.empty())"
	    // << std::endl;

	    tgba_succ_iterator* iter = todo.top().iter;
	    const state* s = todo.top().s;

	    // Nothing more to explore, backtrack.
	    if (iter->done())
	      {
		todo.pop();
		delete iter;
		seen.erase(s);
		if (todo.size())
		  {
		    assert(path.size());
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
	    path.push_back(state_proposition(h_dest,
					     iter->current_condition()));

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
	    if (best_path.size())
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

	// Append our best path to the period.
	for (cycle_path::iterator it = best_path.begin();
	     it != best_path.end(); ++it)
	  {
	    period.push_back(*it);
	    //ce::state_ce ce(it->first->clone(), it->second);
	    //counter_->cycle.push_back(ce);
	    //counter_->cycle.push_back(*it);
	  }

	// Prepare to find another path for the remaining acceptance
	// conditions.
	acc_to_traverse -= best_acc;
	start = period.back().first;
      }

    // Complete the path so that it goes back to its beginning,
    // forming a cycle.
    complete_cycle(scc, start, suffix.back());
  }

  /////////////

  std::ostream&
  counter_example::print_result(std::ostream& os, const tgba* restrict) const
  {
    os << "Prefix:" << std::endl;
    const bdd_dict* d = ecs_->aut->get_dict();
    for (state_sequence::const_iterator i_se = suffix.begin();
	 i_se != suffix.end(); ++i_se)
      {
	os << "  ";
	if (restrict)
	  {
	    const state* s = ecs_->aut->project_state(*i_se, restrict);
	    assert(s);
	    os << restrict->format_state(s) << std::endl;
	    delete s;
	  }
	else
	  {
	    os << ecs_->aut->format_state(*i_se) << std::endl;
	  }
      }
    os << "Cycle:" <<std::endl;
    for (cycle_path::const_iterator it = period.begin();
	 it != period.end(); ++it)
      {
	os << "    | " << bdd_format_set(d, it->second) << std::endl;
	os << "  ";
	if (restrict)
	  {
	    const state* s = ecs_->aut->project_state(it->first, restrict);
	    assert(s);
	    os << restrict->format_state(s) << std::endl;
	    delete s;
	  }
	else
	  {
	    os << ecs_->aut->format_state(it->first) << std::endl;
	  }
      }
    return os;
  }


  void
  counter_example::print_stats(std::ostream& os) const
  {
    ecs_->print_stats(os);
    os << suffix.size() << " states in suffix" << std::endl;
    os << period.size() << " states in period" << std::endl;
  }

  ce::counter_example*
  counter_example::get_counter_example() const
  {
    return counter_;
  }

}
