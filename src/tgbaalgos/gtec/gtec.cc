// Copyright (C) 2003, 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#include "gtec.hh"
#include "ce.hh"

namespace spot
{
  namespace
  {
    typedef std::pair<const spot::state*, tgba_succ_iterator*> pair_state_iter;
  }

  couvreur99_check::couvreur99_check(const tgba* a,
				     const numbered_state_heap_factory* nshf)
  {
    ecs_ = new couvreur99_check_status(a, nshf);
  }

  couvreur99_check::~couvreur99_check()
  {
    delete ecs_;
  }

  void
  couvreur99_check::remove_component(const state* from)
  {
    // Remove from H all states which are reachable from state FROM.

    // Stack of iterators towards states to remove.
    std::stack<tgba_succ_iterator*> to_remove;

    // Remove FROM itself, and prepare to remove its successors.
    // (FROM should be in H, otherwise it means all reachable
    // states from FROM have already been removed and there is no
    // point in calling remove_component.)
    numbered_state_heap::state_index_p spi = ecs_->h->index(from);
    assert(spi.first);
    assert(*spi.second != -1);
    *spi.second = -1;
    tgba_succ_iterator* i = ecs_->aut->succ_iter(from);

    for (;;)
      {
	// Remove each destination of this iterator.
	for (i->first(); !i->done(); i->next())
	  {
	    state* s = i->current_state();
	    numbered_state_heap::state_index_p spi = ecs_->h->index(s);

	    // This state is not necessary in H, because if we were
	    // doing inclusion checking during the emptiness-check
	    // (redefining find()), the index `s' can be included in a
	    // larger state and will not be found by index().  We can
	    // safely ignore such states.
	    if (!spi.first)
	      continue;

	    if (*spi.second != -1)
	      {
		*spi.second = -1;
		to_remove.push(ecs_->aut->succ_iter(spi.first));
	      }
	  }
	delete i;
	if (to_remove.empty())
	  break;
	i = to_remove.top();
	to_remove.pop();
      }
  }

  emptiness_check_result*
  couvreur99_check::check()
  {
    // We use five main data in this algorithm:
    // * couvreur99_check::root, a stack of strongly connected components (SCC),
    // * couvreur99_check::h, a hash of all visited nodes, with their order,
    //   (it is called "Hash" in Couvreur's paper)
    // * arc, a stack of acceptance conditions between each of these SCC,
    std::stack<bdd> arc;
    // * num, the number of visited nodes.  Used to set the order of each
    //   visited node,
    int num = 1;
    // * todo, the depth-first search stack.  This holds pairs of the
    //   form (STATE, ITERATOR) where ITERATOR is a tgba_succ_iterator
    //   over the successors of STATE.  In our use, ITERATOR should
    //   always be freed when TODO is popped, but STATE should not because
    //   it is also used as a key in H.
    std::stack<pair_state_iter> todo;

    // Setup depth-first search from the initial state.
    {
      state* init = ecs_->aut->get_init_state();
      ecs_->h->insert(init, 1);
      ecs_->root.push(1);
      arc.push(bddfalse);
      tgba_succ_iterator* iter = ecs_->aut->succ_iter(init);
      iter->first();
      todo.push(pair_state_iter(init, iter));
    }

    while (!todo.empty())
      {
	assert(ecs_->root.size() == arc.size());

	// We are looking at the next successor in SUCC.
	tgba_succ_iterator* succ = todo.top().second;

	// If there is no more successor, backtrack.
	if (succ->done())
	  {
	    // We have explored all successors of state CURR.
	    const state* curr = todo.top().first;

	    // Backtrack TODO.
	    todo.pop();

	    // When backtracking the root of an SCC, we must also
	    // remove that SCC from the ARC/ROOT stacks.  We must
	    // discard from H all reachable states from this SCC.
	    numbered_state_heap::state_index_p spi = ecs_->h->index(curr);
	    assert(spi.first);
	    assert(!ecs_->root.empty());
	    if (ecs_->root.top().index == *spi.second)
	      {
		assert(!arc.empty());
		arc.pop();
		ecs_->root.pop();
		remove_component(curr);
	      }

	    delete succ;
	    // Do not delete CURR: it is a key in H.
	    continue;
	  }

	// We have a successor to look at.  Fetch the values
	// (destination state, acceptance conditions of the arc)
	// we are interested in...
	const state* dest = succ->current_state();
	bdd acc = succ->current_acceptance_conditions();
	// ... and point the iterator to the next successor, for
	// the next iteration.
	succ->next();
	// We do not need SUCC from now on.

	// Are we going to a new state?
	numbered_state_heap::state_index_p spi = ecs_->h->find(dest);
	if (!spi.first)
	  {
	    // Yes.  Number it, stack it, and register its successors
	    // for later processing.
	    ecs_->h->insert(dest, ++num);
	    ecs_->root.push(num);
	    arc.push(acc);
	    tgba_succ_iterator* iter = ecs_->aut->succ_iter(dest);
	    iter->first();
	    todo.push(pair_state_iter(dest, iter));
	    continue;
	  }

	// If we have reached a dead component, ignore it.
	if (*spi.second == -1)
	  continue;

	// Now this is the most interesting case.  We have reached a
	// state S1 which is already part of a non-dead SCC.  Any such
	// non-dead SCC has necessarily been crossed by our path to
	// this state: there is a state S2 in our path which belongs
	// to this SCC too.  We are going to merge all states between
	// this S1 and S2 into this SCC.
	//
	// This merge is easy to do because the order of the SCC in
	// ROOT is ascending: we just have to merge all SCCs from the
	// top of ROOT that have an index greater to the one of
	// the SCC of S2 (called the "threshold").
	int threshold = *spi.second;
	while (threshold < ecs_->root.top().index)
	  {
	    assert(!ecs_->root.empty());
	    assert(!arc.empty());
	    acc |= ecs_->root.top().condition;
	    acc |= arc.top();
	    ecs_->root.pop();
	    arc.pop();
	  }
	// Note that we do not always have
	//  threshold == ecs_->root.top().index
	// after this loop, the SCC whose index is threshold might have
	// been merged with a lower SCC.

	// Accumulate all acceptance conditions into the merged SCC.
	ecs_->root.top().condition |= acc;

	if (ecs_->root.top().condition
	    == ecs_->aut->all_acceptance_conditions())
	  {
	    // We have found an accepting SCC.
	    // Release all iterators in TODO.
	    while (!todo.empty())
	      {
		delete todo.top().second;
		todo.pop();
	      }
            set_states(ecs_->states());
	    return new couvreur99_check_result(ecs_);
	  }
      }
    // This automaton recognizes no word.
    return 0;
  }

  const couvreur99_check_status*
  couvreur99_check::result() const
  {
    return ecs_;
  }

  std::ostream&
  couvreur99_check::print_stats(std::ostream& os) const
  {
    ecs_->print_stats(os);
    return os;
  }

  //////////////////////////////////////////////////////////////////////

  couvreur99_check_shy::couvreur99_check_shy(const tgba* a,
					     const numbered_state_heap_factory*
					     nshf)
    : couvreur99_check(a, nshf), num(1)
  {
    // Setup depth-first search from the initial state.
    todo.push(pair_state_successors(0, succ_queue()));
    todo.top().second.push_front(successor(bddtrue,
					   ecs_->aut->get_init_state()));
  }

  couvreur99_check_shy::~couvreur99_check_shy()
  {
  }

  emptiness_check_result*
  couvreur99_check_shy::check()
  {

    for (;;)
      {
	assert(ecs_->root.size() == arc.size());

	// Get the successors of the current state.
	succ_queue& queue = todo.top().second;

	// First, we process all successors that we have already seen.
	// This is an idea from Soheib Baarir.  It helps to merge SCCs
	// and get shorter traces faster.
	succ_queue::iterator q = queue.begin();
	while (q != queue.end())
	  {
	    int* i = find_state(q->s);
	    if (!i)
	      {
		// Skip unknown states.
		++q;
		continue;
	      }

	    // Skip states from dead SCCs.
	    if (*i != -1)
	      {
		// Now this is the most interesting case.  We have
		// reached a state S1 which is already part of a
		// non-dead SCC.  Any such non-dead SCC has
		// necessarily been crossed by our path to this
		// state: there is a state S2 in our path which
		// belongs to this SCC too.  We are going to merge
		// all states between this S1 and S2 into this
		// SCC.
		//
		// This merge is easy to do because the order of
		// the SCC in ROOT is ascending: we just have to
		// merge all SCCs from the top of ROOT that have
		// an index greater to the one of the SCC of S2
		// (called the "threshold").
		int threshold = *i;
		bdd acc = q->acc;
		while (threshold < ecs_->root.top().index)
		  {
		    assert(!ecs_->root.empty());
		    assert(!arc.empty());
		    acc |= ecs_->root.top().condition;
		    acc |= arc.top();
		    ecs_->root.pop();
		    arc.pop();
		  }
		// Note that we do not always have
		//  threshold == ecs_->root.top().index
		// after this loop, the SCC whose index is threshold
		// might have been merged with a lower SCC.

		// Accumulate all acceptance conditions into the
		// merged SCC.
		ecs_->root.top().condition |= acc;

		if (ecs_->root.top().condition
		    == ecs_->aut->all_acceptance_conditions())
		  {
		    /// q->s has already been freed by find_state().
		    queue.erase(q);
		    // We have found an accepting SCC.  Clean up TODO.
		    // We must delete all states of apparing in TODO
		    // unless they are used as keys in H.
		    while (!todo.empty())
		      {
			succ_queue& queue = todo.top().second;
			for (succ_queue::iterator q = queue.begin();
			     q != queue.end(); ++q)
			  {
			    // Delete the state if it is a clone of a
			    // state in the heap...
			    numbered_state_heap::state_index_p spi
			      = ecs_->h->index(q->s);
			    // ... or if it is an unknown state.
			    if (spi.first == 0)
			      delete q->s;
			  }
			todo.pop();
		      }
                    set_states(ecs_->states());
		    return new couvreur99_check_result(ecs_);
		  }
	      }
	    // Remove that state from the queue, so we do not
	    // recurse into it.
	    succ_queue::iterator old = q++;
	    queue.erase(old);
	  }

	// If there is no more successor, backtrack.
	if (queue.empty())
	  {
	    // We have explored all successors of state CURR.
	    const state* curr = todo.top().first;
	    // Backtrack TODO.
	    todo.pop();
	    if (todo.empty())
	      // This automaton recognizes no word.
	      return 0;

	    // When backtracking the root of an SCC, we must also
	    // remove that SCC from the ARC/ROOT stacks.  We must
	    // discard from H all reachable states from this SCC.
	    numbered_state_heap::state_index_p spi = ecs_->h->index(curr);
	    assert(spi.first);
	    assert(!ecs_->root.empty());
	    if (ecs_->root.top().index == *spi.second)
	      {
		assert(!arc.empty());
		arc.pop();
		ecs_->root.pop();
		remove_component(curr);
	      }
	    continue;
	  }

	// Recurse.  (Finally!)

	// Pick one successor off the list, and schedule its
	// successors first on TODO.  Update the various hashes and
	// stacks.
	successor succ = queue.front();
	queue.pop_front();
	ecs_->h->insert(succ.s, ++num);
	ecs_->root.push(num);
	arc.push(succ.acc);
	todo.push(pair_state_successors(succ.s, succ_queue()));
	succ_queue& new_queue = todo.top().second;
	tgba_succ_iterator* iter = ecs_->aut->succ_iter(succ.s);
	for (iter->first(); !iter->done(); iter->next())
	  new_queue.push_back(successor(iter->current_acceptance_conditions(),
					iter->current_state()));
	delete iter;
      }
  }

  int*
  couvreur99_check_shy::find_state(const state* s)
  {
    return ecs_->h->find(s).second;
  }

}
