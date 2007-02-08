// Copyright (C) 2003, 2004, 2005, 2006, 2007 Laboratoire d'Informatique de
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
#include "misc/memusage.hh"
#include "tgbaalgos/statepipe.hh"
#include <iostream>
#include <set>
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <errno.h>
#include <fcntl.h>		/* fcntl, O_NONBLOCK */
#include <sys/wait.h>

namespace spot
{
  namespace
  {
    typedef std::pair<const spot::state*, tgba_succ_iterator*> pair_state_iter;
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
    stats["vmsize"] =
	static_cast<spot::unsigned_statistics::unsigned_fun>
	(&couvreur99_check::get_vmsize);
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

  unsigned
  couvreur99_check::get_vmsize() const
  {
    int size = memusage();
    if (size > 0)
      return size;
    return 0;
  }

  void
  couvreur99_check::remove_component(const state* from)
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
	    assert(*spi.second != -1);
	    *spi.second = -1;
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
    assert(*spi.second != -1);
    *spi.second = -1;
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

	    // Backtrack TODO.
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
	    ecs_->root.push(num);
	    arc.push(acc);
	    tgba_succ_iterator* iter = ecs_->aut->succ_iter(dest);
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
	while (threshold < ecs_->root.top().index)
	  {
	    assert(!ecs_->root.empty());
	    assert(!arc.empty());
	    acc |= ecs_->root.top().condition;
	    acc |= arc.top();
	    rem.splice(rem.end(), ecs_->root.rem());
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

	if (ecs_->root.top().condition
	    == ecs_->aut->all_acceptance_conditions())
	  {
	    // We have found an accepting SCC.
	    // Release all iterators in TODO.
	    while (!todo.empty())
	      {
		delete todo.top().second;
		todo.pop();
		dec_depth();
	      }
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
					     couvreur99_check_shy* shy)
	: s(s), n(n)
  {
    tgba_succ_iterator* iter = shy->ecs_->aut->succ_iter(s);
    for (iter->first(); !iter->done(); iter->next(), shy->inc_transitions())
      {
	q.push_back(successor(iter->current_acceptance_conditions(),
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
    onepass_ = o.get("onepass", 0);

    // Setup depth-first search from the initial state.
    const state* i = ecs_->aut->get_init_state();
    ecs_->h->insert(i, ++num);
    ecs_->root.push(num);
    todo.push_back(todo_item(i, num, this));
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
	dec_depth(todo.back().q.size() + 1);
	todo.pop_back();
      }
    dec_depth(ecs_->root.clear_rem());
    assert(depth() == 0);
  }

  emptiness_check_result*
  couvreur99_check_shy::check()
  {
    // Position in the loop seeking known successors.
    pos = todo.back().q.begin();

    for (;;)
      {
	assert(ecs_->root.size() == 1 + arc.size());

	// Get the successors of the current state.
	succ_queue& queue = todo.back().q;

	// If there is no more successor, backtrack.
	if (queue.empty())
	  {

	    // We have explored all successors of state CURR.
	    const state* curr = todo.back().s;
	    int index = todo.back().n;

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
		arc.pop();
		remove_component(curr);
		ecs_->root.pop();
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
	if (onepass_)
	  pos = queue.end();
	if (pos == queue.end())
	  old = queue.begin();
	else
	  old = pos;
	successor succ = *old;
	// Beware: the implementation of find_state in ifage/gspn/ssp.cc
	// uses POS and modify QUEUE.
	numbered_state_heap::state_index_p sip = find_state(succ.s);
	if (pos != queue.end())
	  ++pos;
	int* i = sip.second;

	if (!i)
	  {
	    // It's a new state.
	    // If we are seeking known states, just skip it.
	    if (pos != queue.end())
	      continue;
	    // Otherwise, number it and stack it so we recurse.
	    queue.erase(old);
	    dec_depth();
	    ecs_->h->insert(succ.s, ++num);
	    ecs_->root.push(num);
	    arc.push(succ.acc);
	    todo.push_back(todo_item(succ.s, num, this));
	    pos = todo.back().q.begin();
	    inc_depth();
	    continue;
	  }

	queue.erase(old);
	dec_depth();

	// Skip dead states.
	if (*i == -1)
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
	while (threshold < ecs_->root.top().index)
	  {
	    assert(!ecs_->root.empty());
	    assert(!arc.empty());
	    acc |= ecs_->root.top().condition;
	    acc |= arc.top();
	    rem.splice(rem.end(), ecs_->root.rem());
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

	// Have we found all acceptance conditions?
	if (ecs_->root.top().condition
	    == ecs_->aut->all_acceptance_conditions())
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


  namespace {
    class couvreur99_check_parallel;
    typedef couvreur99_check_parallel** check_tab;

    struct checkpool {
      state_pipe* pipes;
      int size;
    };

    class couvreur99_check_parallel : public couvreur99_check_shy
    {
    public:
      couvreur99_check_parallel
      (int me, checkpool& pool, const tgba* a,
       option_map o = option_map(),
       const numbered_state_heap_factory* nshf
       = numbered_state_heap_hash_map_factory::instance())
	: couvreur99_check_shy(a, o, nshf), me_(me), pool_(pool),
	  items_sent_(0), items_received_(0)
      {
	stats["items received"] =
	  static_cast<spot::unsigned_statistics::unsigned_fun>
	  (&couvreur99_check_parallel::get_items_received);
	stats["items sent"] =
	  static_cast<spot::unsigned_statistics::unsigned_fun>
	  (&couvreur99_check_parallel::get_items_sent);
      }

      unsigned
      get_items_received() const
      {
	return items_received_;
      }

      unsigned
      get_items_sent() const
      {
	return items_sent_;
      }

      virtual ~couvreur99_check_parallel()
      {
      }

      void
      remove_component(const state* from)
      {
	++removed_components;
	// If rem has been updated, removing states is very easy.
	if (poprem_)
	  {
	    assert(!ecs_->root.rem().empty());
	    dec_depth(ecs_->root.rem().size());
	    std::list<const state*>::iterator i;
	    if (pool_.size > 1)
	      broadcast(ecs_->root.rem());
	    for (i = ecs_->root.rem().begin();
		 i != ecs_->root.rem().end(); ++i)
	      {
		numbered_state_heap::state_index_p spi = ecs_->h->index(*i);
		assert(spi.first == *i);
		assert(*spi.second != -1);
		*spi.second = -1;
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
	assert(*spi.second != -1);
	*spi.second = -1;
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

      virtual emptiness_check_result* check()
      {
	// Position in the loop seeking known successors.
	pos = todo.back().q.begin();

	for (;;)
	  {
	    if (pool_.size > 1)
	      process_buffer();

	    assert(ecs_->root.size() == 1 + arc.size());

	    // Get the successors of the current state.
	    succ_queue& queue = todo.back().q;

	    // If there is no more successor, backtrack.
	    if (queue.empty())
	      {
		// We have explored all successors of state CURR.
		const state* curr = todo.back().s;
		int index = todo.back().n;

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
		    arc.pop();
		    remove_component(curr);
		    ecs_->root.pop();
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

	    if (onepass_)
	      pos = queue.end();

	    if (pos == queue.end())
	      old = queue.begin();
	    else
	      old = pos;

	    // Pick a random successor.
	    if (pos == queue.end())
	      {
		int n = rand() % queue.size();
		while (n--)
		  {
		    ++old;
		    assert(old != queue.end());
		  }
	      }

	    successor succ = *old;
	    // Beware: the implementation of find_state in ifage/gspn/ssp.cc
	    // uses POS and modify QUEUE.
	    numbered_state_heap::state_index_p sip = find_state(succ.s);
	    if (pos != queue.end())
	      ++pos;
	    int* i = sip.second;

	    if (!i)
	      {
		// It's a new state.
		// If we are seeking known states, just skip it.
		if (pos != queue.end())
		  continue;
		// Otherwise, number it and stack it so we recurse.
		queue.erase(old);
		dec_depth();
		ecs_->h->insert(succ.s, ++num);
		ecs_->root.push(num);
		arc.push(succ.acc);
		todo.push_back(todo_item(succ.s, num, this));
		pos = todo.back().q.begin();
		inc_depth();
		continue;
	      }

	    queue.erase(old);
	    dec_depth();

	    // Skip dead states.
	    if (*i == -1)
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
	    while (threshold < ecs_->root.top().index)
	      {
		assert(!ecs_->root.empty());
		assert(!arc.empty());
		acc |= ecs_->root.top().condition;
		acc |= arc.top();
		rem.splice(rem.end(), ecs_->root.rem());
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

	    // Have we found all acceptance conditions?
	    if (ecs_->root.top().condition
		== ecs_->aut->all_acceptance_conditions())
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

    private:
      int me_;
      checkpool& pool_;

      void
      broadcast(const std::list<const state*>& data)
      {
	// std::cout << getpid() << ": broadcast" << std::endl;

	for (int i = 0; i < pool_.size; ++i)
	  {
	    if (i == me_)
	      continue;

	    // FIXME: calling serialize once for the whole list would be faster
	    for (std::list<const state*>::const_iterator j = data.begin();
		 j != data.end(); ++j)
	      {
		while (pool_.pipes[i].write_state(*j))
		  process_buffer();
	      }
	  }
	items_sent_ += data.size();
      }

      void
      process_buffer()
      {
	int cpt = 0;
	spot::state* s;

	// std::cout << getpid() << ": pbuffer" << std::endl;

	while ((s = pool_.pipes[me_].read_state(automaton())))
	  {
	    numbered_state_heap::state_index_p sip = find_state(s);
	    int* j = sip.second;

	    if (!j)
	      {
		// It's a new state.
		ecs_->h->insert(s, -1);
	      }
	    else
	      {
		// It's a known state (find_state has already delete it).
		*j = -1;
	      }
	    ++cpt;
	  }

	items_received_ += cpt;
      }
    private:
      unsigned items_sent_;
      unsigned items_received_;
    };


    class couvreur99_check_parallel_proxy: public emptiness_check
    {
    private:
      const numbered_state_heap_factory* nshf_;
      checkpool pool_;
    public:
      couvreur99_check_parallel_proxy
      (const tgba* a,
       option_map o = option_map(),
       const numbered_state_heap_factory* nshf
       = numbered_state_heap_hash_map_factory::instance())
	: emptiness_check(a, o), nshf_(nshf)
      {
	pool_.size = o.get("parallel", 2);
      }

      ~couvreur99_check_parallel_proxy()
      {
      }

      emptiness_check_result* check ()
      {
	pool_.pipes = new state_pipe[pool_.size];

	std::cout << std::flush;
	std::cerr << std::flush;
	std::clog << std::flush;
	// Create pool_.size children.
	int i;
	for (i = 0; i < pool_.size; ++i)
	  {
	    int pid = fork();
	    if (pid == -1)
	      {
		perror("failed to fork");
		abort();
	      }
	    if (pid == 0)
	      break;
	    std::cout << "FATHER: forking as PID " << pid << std::endl;
	  }


	if (i < pool_.size) // This is the ith child: run the emptiness.
	  {
	    std::cout << getpid() << ": " << i << "th child alive"<< std::endl;
	    srand(getpid());
	    // Close unused FDs.
	    for (int j = 0; j < pool_.size; ++j)
	      {
		if (j != i)
		  pool_.pipes[j].close_read_end();
		else
		  pool_.pipes[j].close_write_end();
	      }

	    emptiness_check* ch =
	      new couvreur99_check_parallel(i, pool_,
					    automaton(), options(),
					    nshf_);
	    std::cout << getpid() << ": child " << i
		      << " running" << std::endl;
	    emptiness_check_result* res = ch->check();
	    if (res)
	      std::cout << getpid() << ": not empty" << std::endl;
	    else
	      std::cout << getpid() << ": empty" << std::endl;


	    spot::unsigned_statistics::stats_map::const_iterator i;
	    spot::unsigned_statistics* s =
	      dynamic_cast<spot::unsigned_statistics*>(ch);
	    assert(s != 0);
	    for (i = s->stats.begin(); i != s->stats.end(); ++i)
	      std::cout << i->first << " = " << (s->*i->second)() << std::endl;

	    exit(res != 0);
	  }

	// This is the father process.
	// We don't need these pipes, they were for children.
	delete[] pool_.pipes;

	std::cout << "FATHER: all children running" << std::endl;

	int res = 0;
	for (int n = pool_.size; n > 0; --n)
	  {
	    std::cout << "FATHER: waiting for " << n
		      << " children to finish" << std::endl;
	    int stat;
	    int child = wait(&stat);
	    if (child == -1)
	      {
		perror("failed to wait for children");
		abort();
	      }
	    if (WIFSIGNALED(stat))
	      std::cout << "FATHER: child with PID " << child << " signaled "
			<< WTERMSIG(stat) << std::endl;
	    else
	      {
		std::cout << "FATHER: child with PID " << child
			  << " terminated " << WEXITSTATUS(stat) << std::endl;
		if (n == pool_.size)
		  res = WEXITSTATUS(stat); // 0: empty, 1: not empty
		else
		  // All successful children should agree.
		  assert(res == WEXITSTATUS(stat));
	      }
	  }
	std::cout << "FATHER: everybody's dead" << std::endl;

	if (res)
	  return new emptiness_check_result(automaton(), options());
	else
	  return 0;
      }

    };
  }


  emptiness_check*
  couvreur99(const tgba* a,
	     option_map o,
	     const numbered_state_heap_factory* nshf)
  {
    if (o.get("parallel"))
      return new couvreur99_check_parallel_proxy(a, o, nshf);
    if (o.get("shy"))
      return new couvreur99_check_shy(a, o, nshf);
    return  new couvreur99_check(a, o, nshf);
  }

}
