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

namespace spot
{
  namespace
  {
    typedef std::pair<const spot::state*, tgba_succ_iterator*> pair_state_iter;
  }

  emptiness_check::emptiness_check(const tgba* a,
				   const numbered_state_heap_factory* nshf)
  {
    ecs_ = new emptiness_check_status(a, nshf);
  }

  emptiness_check::~emptiness_check()
  {
    delete ecs_;
  }

  void
  emptiness_check::remove_component(const state* from)
  {
    // Remove from H all states which are reachable from state FROM.

    // Stack of iterators towards states to remove.
    std::stack<tgba_succ_iterator*> to_remove;

    // Remove FROM itself, and prepare to remove its successors.
    // (FROM should be in H, otherwise it means all reachable
    // states from FROM have already been removed and there is no
    // point in calling remove_component.)
    int* hi = ecs_->h->find(from);
    assert(hi);
    assert(*hi != -1);
    *hi = -1;
    tgba_succ_iterator* i = ecs_->aut->succ_iter(from);

    for (;;)
      {
	// Remove each destination of this iterator.
	for (i->first(); !i->done(); i->next())
	  {
	    state* s = i->current_state();
	    int *hi = ecs_->h->find(s);
	    assert(hi);

	    if (*hi != -1)
	      {
		*hi = -1;
		to_remove.push(ecs_->aut->succ_iter(s));
	      }
	    delete s;
	  }
	delete i;
	if (to_remove.empty())
	  break;
	i = to_remove.top();
	to_remove.pop();
      }
  }

  bool
  emptiness_check::check()
  {
    // We use five main data in this algorithm:
    // * emptiness_check::root, a stack of strongly connected components (SCC),
    // * emptiness_check::h, a hash of all visited nodes, with their order,
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
	    int* i = ecs_->h->find(curr);
	    assert(i);
	    assert(!ecs_->root.empty());
	    if (ecs_->root.top().index == *i)
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
	int* i = ecs_->h->find(dest);
	if (!i)
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

	// We know the state exists.  Since a state can have several
	// representations (i.e., objects), make sure we delete
	// anything but the first one seen (the one used as key in H).
	(void) ecs_->h->filter(dest);

	// If we have reached a dead component, ignore it.
	if (*i == -1)
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
	int threshold = *i;
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
	    return false;
	  }
      }
    // This automaton recognizes no word.
    return true;
  }

  const emptiness_check_status*
  emptiness_check::result() const
  {
    return ecs_;
  }

  //////////////////////////////////////////////////////////////////////

  emptiness_check_shy::emptiness_check_shy(const tgba* a,
					   const numbered_state_heap_factory*
					   nshf)
    : emptiness_check(a, nshf)
  {
  }

  emptiness_check_shy::~emptiness_check_shy()
  {
  }

  struct successor {
    bdd acc;
    const spot::state* s;
    successor(bdd acc, const spot::state* s): acc(acc), s(s) {}
  };

  bool
  emptiness_check_shy::check()
  {
    // We use five main data in this algorithm:
    // * emptiness_check::root, a stack of strongly connected components (SCC),
    // * emptiness_check::h, a hash of all visited nodes, with their order,
    //   (it is called "Hash" in Couvreur's paper)
    // * arc, a stack of acceptance conditions between each of these SCC,
    std::stack<bdd> arc;
    // * num, the number of visited nodes.  Used to set the order of each
    //   visited node,
    int num = 1;
    // * todo, the depth-first search stack.  This holds pairs of the
    //   form (STATE, SUCCESSORS) where SUCCESSORS is a list of
    //   (ACCEPTANCE_CONDITIONS, STATE) pairs.
    typedef std::list<successor> succ_queue;
    typedef std::pair<const state*, succ_queue> pair_state_successors;
    std::stack<pair_state_successors> todo;

    // Setup depth-first search from the initial state.
    todo.push(pair_state_successors(0, succ_queue()));
    todo.top().second.push_front(successor(bddtrue,
					   ecs_->aut->get_init_state()));

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
	    int* i = ecs_->h->find(q->s);
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
		    // We have found an accepting SCC.  Clean up TODO.
		    // We must delete all states of apparing in TODO
		    // unless they are used as keys in H.
		    while (!todo.empty())
		      {
			succ_queue& queue = todo.top().second;
			for (succ_queue::iterator q = queue.begin();
			     q != queue.end(); ++q)
			  {
			    int* i = ecs_->h->find(q->s);
			    if (!i)
			      delete q->s;
			    else
			      // Delete the state if it is a clone
			      // of a state in the heap.
			      (void) ecs_->h->filter(q->s);
			  }
			todo.pop();
		      }
		    return false;
		  }
	      }
	    // We know the state exists.  Since a state can have several
	    // representations (i.e., objects), make sure we delete
	    // anything but the first one seen (the one used as key in H).
	    (void) ecs_->h->filter(q->s);
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
	      return true;

	    // When backtracking the root of an SCC, we must also
	    // remove that SCC from the ARC/ROOT stacks.  We must
	    // discard from H all reachable states from this SCC.
	    int* i = ecs_->h->find(curr);
	    assert(i);
	    assert(!ecs_->root.empty());
	    if (ecs_->root.top().index == *i)
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
	for (iter->first(); ! iter->done(); iter->next())
	  new_queue.push_back(successor(iter->current_acceptance_conditions(),
					iter->current_state()));
	delete iter;
      }
  }
}
