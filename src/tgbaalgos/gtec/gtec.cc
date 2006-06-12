// Copyright (C) 2003, 2004, 2005, 2006 Laboratoire d'Informatique de
// Paris 6 (LIP6), département Systèmes Répartis Coopératifs (SRC),
// Université Pierre et Marie Curie.
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
#include "tgba/proviso.hh"
#include "tgba/bddprint.hh"

namespace spot
{
  namespace
  {
    typedef std::pair<const spot::state*, tgba_succ_iterator*> pair_state_iter;
  }

  bool
  couvreur99_check::found_acc() const
  {
    bdd acc = bddtrue;
    bdd curcond = ecs_->root.top().condition;
    for (streett_acceptance_conditions::acc_list::const_iterator i
	   = streett_acc.begin(); i != streett_acc.end(); ++i)
      {
	// Büchi
	if (i->l == bddfalse)
	  return ((ecs_->cycle_acc = curcond) == i->u);
	// Streett
	if ((i->l & curcond) == bddfalse)
	  acc &= !i->l & !i->u;
	else
	  if ((i->u & curcond) == bddfalse)
	    return false;
      }
    ecs_->cycle_acc = ecs_->aut->all_acceptance_conditions() & acc;
    return true;
  }

  couvreur99_check::couvreur99_check(const tgba* a,
				     option_map o,
				     const numbered_state_heap_factory* nshf)
    : emptiness_check(a, o),
      removed_components(0)
  {
    poprem_ = o.get("poprem", 1);
    ecs_ = new couvreur99_check_status(a, nshf);
    stats["removed components"] =
	static_cast<spot::unsigned_statistics::unsigned_fun>
	(&couvreur99_check::get_removed_components);

    const streett_acceptance_conditions* st =
      dynamic_cast<const streett_acceptance_conditions*>(a);
    if (st)
      {
	streett_acc = st->get_streett_acceptance_conditions();
      }
    else
      {
	streett_acc.push_back(streett_pair
			     (bddfalse,
			      ecs_->aut->all_acceptance_conditions()));
      }
  }

  couvreur99_check::~couvreur99_check()
  {
    delete ecs_;
  }

  unsigned
  couvreur99_check::get_removed_components() const
  {
    return removed_components;
  }

  void
  couvreur99_check::remove_component(const state* from, bool del)
  {
    ++removed_components;

    // If rem has been updated, removing states is very easy.
    if (poprem_)
      {
	assert(!ecs_->root.rem().empty());
	dec_depth(ecs_->root.rem().size());
	std::list<const state*>::iterator i;
	for (i = ecs_->root.rem().begin(); i != ecs_->root.rem().end(); ++i)
	  {
	    numbered_state_heap::state_index_p spi = ecs_->h->index(*i);
	    assert(spi.first == *i);
	    assert(*spi.second > -1);
	    if (!del)
	      {
		*spi.second = -1;
	      }
	    else
	      {
		*spi.second = -2;
	      }
	  }
	// ecs_->root.rem().clear();
	return;
      }

    // Remove from H all states which are reachable from state FROM.

    // Stack of iterators towards states to remove.
    std::stack<tgba_succ_iterator*> to_remove;

    // Remove FROM itself, and prepare to remove its successors.
    // (FROM should be in H, otherwise it means all reachable
    // states from FROM have already been removed and there is no
    // point in calling remove_component.)
    numbered_state_heap::state_index_p spi = ecs_->h->index(from);
    assert(spi.first == from);
    assert(*spi.second > -1);
    *spi.second = (del ? -2 : -1);
    tgba_succ_iterator* i = ecs_->aut->succ_iter(from);

    for (;;)
      {
	// Remove each destination of this iterator.
	for (i->first(); !i->done(); i->next())
	  {
	    inc_transitions();

	    state* s = i->current_state();
	    numbered_state_heap::state_index_p spi = ecs_->h->index(s);

	    // This state is not necessary in H, because if we were
	    // doing inclusion checking during the emptiness-check
	    // (redefining find()), the index `s' can be included in a
	    // larger state and will not be found by index().  We can
	    // safely ignore such states.
	    if (!spi.first)
	      continue;

	    if (*spi.second > -1)
	      {
		*spi.second = (del ? -2 : -1);
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
      tgba_succ_iterator* iter = ecs_->aut->succ_iter(init);
      ecs_->root.push(1, iter->get_proviso());
      arc.push(bddfalse);
      iter->first();
      todo.push(pair_state_iter(init, iter));
      inc_depth();
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

	    // Wait!  Maybe we do not really want to backtrack yet.
	    // If we have been using stubborn sets its possible there
	    // are transitions that we have always ignored in this
	    // component.  We want to visit some of these until we
	    // have ignored nobody.  (When stubborn sets are not used,
	    // empty() will always return true.)
	    if (!ecs_->root.top().ignored->empty())
	      {
		tgba_succ_iterator* iter
		  = ecs_->root.top().ignored->oneset(curr, ecs_->aut);
		iter->first();
		ecs_->aut->release_proviso(ecs_->root.top().ignored);
		 ecs_->root.top().ignored = iter->get_proviso();
		todo.top().second = iter;
		delete succ;
		continue;
	      }

	    // Now really backtrack TODO.
	    todo.pop();
	    dec_depth();

	    // If poprem is used, fill rem with any component removed,
	    // so that remove_component() does not have to traverse
	    // the SCC again.
	    numbered_state_heap::state_index_p spi = ecs_->h->index(curr);
	    assert(spi.first);
	    if (poprem_)
	      {
		ecs_->root.rem().push_front(spi.first);
		inc_depth();
	      }
	    // When backtracking the root of an SCC, we must also
	    // remove that SCC from the ARC/ROOT stacks.  We must
	    // discard from H all reachable states from this SCC.
	    assert(!ecs_->root.empty());
	    if (ecs_->root.top().index == *spi.second)
	      {
		assert(!arc.empty());
		arc.pop();
		remove_component(curr);
		ecs_->aut->release_proviso(ecs_->root.top().ignored);
		ecs_->root.pop();
	      }

	    delete succ;
	    // Do not delete CURR: it is a key in H.
	    continue;
	  }

	// We have a successor to look at.
	inc_transitions();
	// Fetch the values (destination state, acceptance conditions
	// of the arc) we are interested in...
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
	    tgba_succ_iterator* iter = ecs_->aut->succ_iter(dest);
	    ecs_->root.push(num, iter->get_proviso());
	    arc.push(acc);
	    iter->first();
	    todo.push(pair_state_iter(dest, iter));
	    inc_depth();
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
	std::list<const state*> rem;
	proviso* p = 0;
	while (threshold < ecs_->root.top().index)
	  {
	    assert(!ecs_->root.empty());
	    assert(!arc.empty());
	    acc |= ecs_->root.top().condition;
	    acc |= arc.top();
	    rem.splice(rem.end(), ecs_->root.rem());
	    if (!p)
	      {
		p = ecs_->root.top().ignored;
	      }
	    else
	      {
		proviso* p2 = ecs_->root.top().ignored;
		p->intersect(p2);
		ecs_->aut->release_proviso(p2);
	      }
	    ecs_->root.pop();
	    arc.pop();
	  }
	// Note that we do not always have
	//  threshold == ecs_->root.top().index
	// after this loop, the SCC whose index is threshold might have
	// been merged with a lower SCC.

	// Accumulate all acceptance conditions into the merged SCC.
	ecs_->root.top().condition |= acc;
	ecs_->root.rem().splice(ecs_->root.rem().end(), rem);
	if (p)
	  {
	    ecs_->root.top().ignored->intersect(p);
	    ecs_->aut->release_proviso(p);
	  }

	if (found_acc())
	  {
	    // We have found an accepting SCC.
	    // Release all iterators in TODO.
	    while (!todo.empty())
	      {
		delete todo.top().second;
		todo.pop();
		dec_depth();
	      }
	    ecs_->root.release_provisos(ecs_->aut);
	    // Use this state to start the computation of an accepting
	    // cycle.
	    ecs_->cycle_seed = spi.first;
            set_states(ecs_->states());
	    return new couvreur99_check_result(ecs_, options());
	  }
      }
    // This automaton recognizes no word.
    set_states(ecs_->states());
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
    os << transitions() << " transitions explored" << std::endl;
    os << max_depth() << " items max in DFS search stack" << std::endl;
    return os;
  }

  //////////////////////////////////////////////////////////////////////

  couvreur99_check_shy::todo_item::todo_item(const state* s, int n,
					     tgba_succ_iterator* iter,
					     couvreur99_check_shy* shy,
					     bdd avoid)
	: s(s), n(n)
  {
    for (iter->first(); !iter->done(); iter->next(), shy->inc_transitions())
      {
	bdd acc = iter->current_acceptance_conditions();
	((acc - avoid == acc) ? q : qf)
	  .push_back(successor(iter->current_acceptance_conditions(),
			       iter->current_state()));
	shy->inc_depth();
      }
    delete iter;
  }

  couvreur99_check_shy::couvreur99_check_shy(const tgba* a,
					     option_map o,
					     const numbered_state_heap_factory*
					     nshf)
    : couvreur99_check(a, o, nshf), num(1)
  {
    group_ = o.get("group", 1);
    group2_ = o.get("group2", 0);
    group_ |= group2_;

    // Setup depth-first search from the initial state.
    const state* i = ecs_->aut->get_init_state();
    ecs_->h->insert(i, num);
    min.push_back(num);
    avoid.push_back(std::pair<int, bdd>(num, bddfalse));

    tgba_succ_iterator* iter = ecs_->aut->succ_iter(i);
    ecs_->root.push(num, iter->get_proviso());
    todo.push_back(todo_item(i, num, iter, this, bddfalse));
    inc_depth(1);
  }

  couvreur99_check_shy::~couvreur99_check_shy()
  {
  }

  void
  couvreur99_check_shy::clear_todo()
  {
    // We must delete all states appearing in TODO
    // unless they are used as keys in H.
    while (!todo.empty())
      {
	{
	  succ_queue& queue = todo.back().q;
	  for (succ_queue::iterator q = queue.begin();
	       q != queue.end(); ++q)
	    {
	      // Delete the state if it is a clone of a
	      // state in the heap...
	      numbered_state_heap::state_index_p spi = ecs_->h->index(q->s);
	      // ... or if it is an unknown state.
	      if (spi.first == 0)
		delete q->s;
	    }
	}
	{
	  succ_queue& queue = todo.back().qf;
	  for (succ_queue::iterator q = queue.begin();
	       q != queue.end(); ++q)
	    {
	      // Delete the state if it is a clone of a
	      // state in the heap...
	      numbered_state_heap::state_index_p spi = ecs_->h->index(q->s);
	      // ... or if it is an unknown state.
	      if (spi.first == 0)
		delete q->s;
	    }
	}
	dec_depth(todo.back().q.size() + todo.back().qf.size() + 1);
	todo.pop_back();
      }
    dec_depth(ecs_->root.clear_rem());
    ecs_->root.release_provisos(ecs_->aut);
    assert(depth() == 0);
  }

  emptiness_check_result*
  couvreur99_check_shy::check()
  {
    // Position in the loop seeking known successors.
    succ_queue::iterator pos = todo.back().q.begin();

    for (;;)
      {
	assert(ecs_->root.size() == 1 + arc.size());

	// Get the successors of the current state.
	succ_queue& queue = todo.back().q;

	if (queue.empty() && !todo.back().qf.empty())
	  {
	    queue.splice(queue.begin(), todo.back().qf);
	    min.push_back(num);
	  }

	// If there is no more successor, backtrack.
	if (queue.empty())
	  {
	    // Wait!  Maybe we do not really want to backtrack yet.
	    // If we have been using stubborn sets its possible there
	    // are transitions that we have always ignored in this
	    // component.  We want to visit some of these until we
	    // have ignored nobody.  (When stubborn sets are not used,
	    // empty() will always return true.)
	    if (!ecs_->root.top().ignored->empty())
	      {
		tgba_succ_iterator* iter
		  = ecs_->root.top().ignored->oneset(todo.back().s, ecs_->aut);
		iter->first();
		ecs_->aut->release_proviso(ecs_->root.top().ignored);
		ecs_->root.top().ignored = iter->get_proviso();
		todo.back() = todo_item(todo.back().s, todo.back().n,
					iter, this, bddfalse);
		pos = todo.back().q.begin();
		continue;
	      }

	    // We have explored all successors of state CURR.
	    const state* curr = todo.back().s;
	    int index = todo.back().n;

	    if (index == min.back())
	      min.pop_back();

	    // Backtrack TODO.
	    todo.pop_back();
	    dec_depth();

	    if (todo.empty())
	      {
	    	 // This automaton recognizes no word.
	    	 set_states(ecs_->states());
	    	 assert(poprem_ || depth() == 0);
	    	 return 0;
	      }

	    pos = todo.back().q.begin();

	    // If poprem is used, fill rem with any component removed,
	    // so that remove_component() does not have to traverse
	    // the SCC again.
	    if (poprem_)
	      {
		numbered_state_heap::state_index_p spi = ecs_->h->index(curr);
		assert(spi.first);
		ecs_->root.rem().push_front(spi.first);
		inc_depth();
	      }

	    // When backtracking the root of an SCC, we must also
	    // remove that SCC from the ARC/ROOT stacks.  We must
	    // discard from H all reachable states from this SCC.
	    assert(!ecs_->root.empty());
	    if (ecs_->root.top().index == index)
	      {
		assert(!arc.empty());
		bdd la = arc.top();
		bdd acc = ecs_->root.top().condition;
		bdd old_avoid = avoid.back().second;
		if (avoid.back().first == index)
		  avoid.pop_back();
		bdd new_avoid = old_avoid;
		arc.pop();
		for (streett_acceptance_conditions::acc_list::const_iterator
		       i = streett_acc.begin(); i != streett_acc.end(); ++i)
		  {
		    if (((i->u & acc) == bddfalse)
			&& ((i->l & acc) != bddfalse))
		      {
			new_avoid |= i->l;
		      }
		  }

		ecs_->aut->release_proviso(ecs_->root.top().ignored);

		if (new_avoid != old_avoid)
		  {
		    avoid.push_back(std::pair<int, bdd>(index, new_avoid));

		    remove_component(curr, true);
		    ecs_->root.pop();
		    ecs_->h->insert(curr, index);
		    num = index + 1;
		    tgba_succ_iterator* iter = ecs_->aut->succ_iter(curr);
		    ecs_->root.push(num, iter->get_proviso());
		    arc.push(la);
		    todo.push_back(todo_item(curr, num, iter, this,
					     new_avoid));
		    pos = todo.back().q.begin();
		    inc_depth();
		  }
		else
		  {
		    remove_component(curr);
		    ecs_->root.pop();
		  }
	      }
	    continue;
	  }

	// We always make a first pass over the successors of a state
	// to check whether it contains some state we have already seen.
	// This way we hope to merge the most SCCs before stacking new
	// states.
	//
	// So are we checking for known states ?  If yes, POS tells us
	// which state we are considering.  Otherwise just pick the
	// first one.
	succ_queue::iterator old;
	if (pos == queue.end())
	  old = queue.begin();
	else
	  old = pos++;
	successor succ = *old;

	numbered_state_heap::state_index_p sip = find_state(succ.s);
	int* i = sip.second;

	if (!i || *i == -2)
	  {
	    // It's a new state.
	    // If we are seeking known states, just skip it.
	    if (pos != queue.end())
	      continue;
	    // Otherwise, number it and stack it so we recurse.
	    queue.erase(old);
	    dec_depth();
	    const state* s = i ? sip.first : succ.s;
	    ecs_->h->insert(s, ++num);
	    tgba_succ_iterator* iter = ecs_->aut->succ_iter(s);
	    ecs_->root.push(num, iter->get_proviso());
	    arc.push(succ.acc);
	    todo.push_back(todo_item(s, num, iter, this,
				     avoid.back().second));
	    pos = todo.back().q.begin();
	    inc_depth();
	    continue;
	  }

	queue.erase(old);
	dec_depth();

	// Skip dead states.
	if (*i == -1)
	  continue;

	// ignore "avoided" transitions
	if (*i <= min.back())
	  continue;

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
	std::list<const state*> rem;
	bdd acc = succ.acc;
	proviso* p = 0;
	while (threshold < ecs_->root.top().index)
	  {
	    assert(!ecs_->root.empty());
	    assert(!arc.empty());
	    acc |= ecs_->root.top().condition;
	    acc |= arc.top();
	    rem.splice(rem.end(), ecs_->root.rem());
	    if (!p)
	      {
		p = ecs_->root.top().ignored;
	      }
	    else
	      {
		proviso* p2 = ecs_->root.top().ignored;
		p->intersect(p2);
		ecs_->aut->release_proviso(p2);
	      }
	    ecs_->root.pop();
	    arc.pop();
	  }
	// Note that we do not always have
	//   threshold == ecs_->root.top().index
	// after this loop, the SCC whose index is threshold
	// might have been merged with a lower SCC.

	// Accumulate all acceptance conditions into the
	// merged SCC.
	ecs_->root.top().condition |= acc;
	ecs_->root.rem().splice(ecs_->root.rem().end(), rem);
	if (p)
	  {
	    ecs_->root.top().ignored->intersect(p);
	    ecs_->aut->release_proviso(p);
	  }

	// Have we found all acceptance conditions?
	if (found_acc())
	  {
	    // Use this state to start the computation of an accepting
	    // cycle.
	    ecs_->cycle_seed = sip.first;

	    // We have found an accepting SCC.  Clean up TODO.
	    clear_todo();
	    set_states(ecs_->states());
	    return new couvreur99_check_result(ecs_, options());
	  }
	// Group the pending successors of formed SCC if requested.
	if (group_)
	  {
	    assert(todo.back().s);
	    while (ecs_->root.top().index < todo.back().n)
	      {
		todo_list::reverse_iterator prev = todo.rbegin();
		todo_list::reverse_iterator last = prev++;
		// If group2 is used we insert the last->q in front
		// of prev->q so that the states in prev->q are checked
		// for existence again after we have done with the states
		// of last->q.  Otherwise we just append to the end.
		prev->q.splice(group2_ ? prev->q.begin() : prev->q.end(),
			       last->q);

		if (poprem_)
		  {
		    numbered_state_heap::state_index_p spi =
		      ecs_->h->index(todo.back().s);
		    assert(spi.first);
		    ecs_->root.rem().push_front(spi.first);
		    // Don't change the stack depth, since
		    // we are just moving the state from TODO to REM.
		  }
		else
		  {
		    dec_depth();
		  }
		todo.pop_back();
	      }
	    pos = todo.back().q.begin();
	  }
      }
  }

  numbered_state_heap::state_index_p
  couvreur99_check_shy::find_state(const state* s)
  {
    return ecs_->h->find(s);
  }

  emptiness_check*
  couvreur99(const tgba* a,
	     option_map o,
	     const numbered_state_heap_factory* nshf)
  {
    if (o.get("shy"))
      return new couvreur99_check_shy(a, o, nshf);
    return  new couvreur99_check(a, o, nshf);
  }

}
