#include "emptinesscheck.hh"
#include "tgba/bddprint.hh"
#include <queue>
#include <stdio.h>
#include <vector>
#include <map>

namespace spot
{
  typedef std::pair<const spot::state*, tgba_succ_iterator*> pair_state_iter;
  typedef std::pair<pair_state_iter, bdd> triplet;

  connected_component::connected_component(int i)
  {
    index = i;
    condition = bddfalse;
  }

  bool
  connected_component::has_state(const state* s) const
  {
    return state_set.find(s) != state_set.end();
  }


  emptiness_check::emptiness_check(const tgba* a)
    : aut_(a)
  {
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
    hash_type::iterator hi = h.find(from);
    assert(hi->second != -1);
    hi->second = -1;
    tgba_succ_iterator* i = aut_->succ_iter(from);

    for (;;)
      {
	// Remove each destination of this iterator.
	for (i->first(); !i->done(); i->next())
	  {
	    state* s = i->current_state();
	    hash_type::iterator hi = h.find(s);
	    assert(hi != h.end());

	    if (hi->second != -1)
	      {
		hi->second = -1;
		to_remove.push(aut_->succ_iter(s));
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
    // * emptiness_check::h, a hash of all visited node, with their order,
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
      state* init = aut_->get_init_state();
      h[init] = 1;
      root.push(connected_component(1));
      arc.push(bddfalse);
      tgba_succ_iterator* iter = aut_->succ_iter(init);
      iter->first();
      todo.push(pair_state_iter(init, iter));
    }

    while (!todo.empty())
      {
	assert(root.size() == arc.size());

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
	    hash_type::iterator i = h.find(curr);
	    assert(i != h.end());
	    assert(!root.empty());
	    if (root.top().index == i->second)
	      {
		assert(!arc.empty());
		arc.pop();
		root.pop();
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
	bdd acc = succ->current_accepting_conditions();
	// ... and point the iterator to the next successor, for
	// the next iteration.
	succ->next();
	// We do not need SUCC from now on.

	// Are we going to a new state?
	hash_type::iterator i = h.find(dest);
	if (i == h.end())
	  {
	    // Yes.  Number it, stack it, and register its successors
	    // for later processing.
	    h[dest] = ++num;
	    root.push(connected_component(num));
	    arc.push(acc);
	    tgba_succ_iterator* iter = aut_->succ_iter(dest);
	    iter->first();
	    todo.push(pair_state_iter(dest, iter));
	    continue;
	  }

	// We know the state exist.  Since a state can have several
	// representations (i.e., objects), make sure we delete
	// anything but the first one seen (the one used as key in
	// H).
	if (dest != i->first)
	  delete dest;

	// If we have reached a dead component.  Ignore it.
	if (i->second == -1)
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
	int threshold = i->second;
	while (threshold < root.top().index)
	  {
	    assert(!root.empty());
	    assert(!arc.empty());
	    acc |= root.top().condition;
	    acc |= arc.top();
	    root.pop();
	    arc.pop();
	  }
	// Note that we do not always have
	//  threshold == root.top().index
	// after this loop, the SCC whose index is threshold might have
	// been merged with a lower SCC.

	// Accumulate all acceptance conditions into the merged SCC.
	root.top().condition |= acc;

	if (root.top().condition == aut_->all_accepting_conditions())
	  // We have found an accepting SCC.
	  return false;
      }
    // This automaton recognize no word.
    return true;
  }


  std::ostream&
  emptiness_check::print_result(std::ostream& os, const tgba* restrict) const
  {
    os << "Prefix:" << std::endl;
    const bdd_dict* d = aut_->get_dict();
    for (state_sequence::const_iterator i_se = suffix.begin();
	 i_se != suffix.end(); ++i_se)
      {
	os << "  ";
	if (restrict)
	  {
	    const state* s = aut_->project_state(*i_se, restrict);
	    assert(s);
	    os << restrict->format_state(s) << std::endl;
	    delete s;
	  }
	else
	  {
	    os << aut_->format_state(*i_se) << std::endl;
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
	    const state* s = aut_->project_state(it->first, restrict);
	    assert(s);
	    os << restrict->format_state(s) << std::endl;
	    delete s;
	  }
	else
	  {
	    os << aut_->format_state(it->first) << std::endl;
	  }
      }
    return os;
  }

  void
  emptiness_check::counter_example()
  {
    assert(!root.empty());

    int comp_size = root.size();
    // Transform the stack of connected component into an array.
    connected_component* scc = new connected_component[comp_size];
    for (int j = comp_size - 1; 0 <= j; --j)
      {
	scc[j] = root.top();
	root.pop();
      }
    assert(root.empty());

    // Build the set of states for all SCCs.
    for (hash_type::iterator i = h.begin(); i != h.end(); ++i)
      {
	int index = i->second;
	// Skip states from dead SCCs.
	if (index < 0)
	  continue;
	assert(index != 0);

	// Find the SCC this state belongs to.
	int j;
	for (j = 1; j < comp_size; ++j)
	  if (index < scc[j].index)
	    break;
	scc[j - 1].state_set.insert(i->first);
      }

    // seqs[i] is a sequence between SCC i and SCC i+1.
    state_sequence* seqs = new state_sequence[comp_size - 1];
    // FIFO for the breadth-first search.
    std::deque<pair_state_iter> todo;

    // Record the father of each state, while performing the BFS.
    typedef std::map<const state*, const state*,
                     state_ptr_less_than> father_map;
    father_map father;

    state_sequence seq;
    state_sequence tmp_lst;
    state_sequence best_lst;
    bdd tmp_acc = bddfalse;

    state* start_state = aut_->get_init_state();
    if (comp_size != 1)
      {
	tgba_succ_iterator* i = aut_->succ_iter(start_state);
	todo.push_back(pair_state_iter(start_state, i));

	for (int k = 0; k < comp_size - 1; ++k)
	  {
	    // We build a path trought all SCC in the stack: a
	    // possible prefix for a counter example.
	    while (!todo.empty())
	      {
		pair_state_iter started_from = todo.front();
		todo.pop_front();

		for (started_from.second->first();
		     !started_from.second->done();
		     started_from.second->next())
		  {
		    const state* curr_state =
		      started_from.second->current_state();
		    if (scc[k+1].has_state(curr_state))
		      {
			const state* curr_father = started_from.first;
			seq.push_front(curr_state);
			seq.push_front(curr_father);
			hash_type::iterator i_2 = h.find(curr_father);
			assert(i_2 != h.end());
			while (scc[k].index < i_2->second)
			  {
			    assert(i_2->second != 1);
			    assert(father.find(curr_father) != father.end());
			    const state* f = father[curr_father];
			    seq.push_front(f);
			    curr_father = f;
			    i_2 = h.find(curr_father);
			    assert(i_2 != h.end());
			  }
			seqs[k] = seq;
			seq.clear();
			todo.clear();
			break;
		      }
		    else
		      {
			if (scc[k].has_state(curr_state))
			  {
			    father_map::iterator i_path =
			      father.find(curr_state);
			    hash_type::iterator i_seen = h.find(curr_state);

			    if (i_seen != h.end()
				&& i_seen->second > 0
				&& i_path == father.end())
			      {
				todo.
				  push_back(pair_state_iter(curr_state,
							    aut_->succ_iter(curr_state)));
				father[curr_state] = started_from.first;
			      }
			  }
		      }
		  }
	      }
	    assert(!seqs[k].empty());
	    todo.
	      push_back(pair_state_iter(seqs[k].back(),
					aut_->succ_iter(seqs[k].back())));
	  }
      }
    else
      {
	suffix.push_front(start_state);
      }
    for (int n_ = 0; n_ < comp_size - 1; ++n_)
      for (state_sequence::iterator it = seqs[n_].begin();
	   it != seqs[n_].end(); ++it)
	suffix.push_back(*it);
    suffix.unique();
    accepting_path(scc[comp_size - 1], suffix.back(),
		   scc[comp_size - 1].condition);

    delete[] scc;
    delete[] seqs;
  }

  void
  emptiness_check::complete_cycle(const connected_component& comp_path,
				  const state* from_state,
				  const state* to_state)
  {
    if (h[from_state] != h[to_state])
      {
	std::map<const state*, state_proposition,
	         state_ptr_less_than> complete_map;
	std::deque<pair_state_iter> todo_complete;
	tgba_succ_iterator* ite = aut_->succ_iter(from_state);
	todo_complete.push_back(pair_state_iter(from_state, ite));
	cycle_path tmp_comp;
	while(!todo_complete.empty())
	  {
	    pair_state_iter started_ = todo_complete.front();
	    todo_complete.pop_front();
	    tgba_succ_iterator* iter_s = started_.second;
	    iter_s->first();
	    for (iter_s->first(); !iter_s->done(); iter_s->next())
	      {
		const state* curr_state = started_.second->current_state();
		if (comp_path.has_state(curr_state))
		  {
		    if (curr_state->compare(to_state) == 0)
		      {
			const state* curr_father = started_.first;
			bdd curr_condition = iter_s->current_condition();
			tmp_comp.push_front(state_proposition(curr_state, curr_condition));
			while (curr_father->compare(from_state) != 0)
			  {
			    tmp_comp.push_front(state_proposition(curr_father,
						 complete_map[curr_father].second));
			    curr_father = complete_map[curr_father].first;
			  }
			period.splice(period.end(), tmp_comp);
			todo_complete.clear();
			break;
		      }
		    else
		      {
			todo_complete.push_back(pair_state_iter(curr_state,
								aut_->succ_iter(curr_state)));
			complete_map[curr_state] =
			  state_proposition(started_.first,
					    iter_s->current_condition());
		      }
		  }
	      }
	  }
      }
  }

  // FIXME: Derecursive this function.
  void
  emptiness_check::accepting_path(const connected_component& comp_path,
				  const state* start_path, bdd to_accept)
  {
    hash_type seen_priority;
    std::stack<triplet> todo_path;
    tgba_succ_iterator* t_s_i = aut_->succ_iter(start_path);
    t_s_i->first();
    todo_path.push(triplet(pair_state_iter(start_path, t_s_i), bddfalse));
    bdd tmp_acc = bddfalse;
    bdd best_acc = bddfalse;
    cycle_path tmp_lst;
    cycle_path best_lst;
    bool ok = false;
    seen_priority[start_path] = h[start_path];
    while (!todo_path.empty())
      {
	triplet step_ = todo_path.top();
	tgba_succ_iterator* iter_ = step_.first.second;
	if (iter_->done())
	  {
	    todo_path.pop();
	    seen_priority.erase(step_.first.first);
	    tmp_lst.pop_back();
	  }
	else
	  {
	    state* curr_state = iter_->current_state();
	    if (comp_path.has_state(curr_state))
	      {
		hash_type::iterator i = seen_priority.find(curr_state);
		if (i == seen_priority.end())
		  {
		    tgba_succ_iterator* c_iter = aut_->succ_iter(curr_state);
		    bdd curr_bdd =
		      iter_->current_accepting_conditions() | step_.second;
		    c_iter->first();
		    todo_path.push(triplet(pair_state_iter(curr_state, c_iter),
					   curr_bdd));
		    tmp_lst.push_back(state_proposition(curr_state,
							iter_->current_condition()));
		    seen_priority[curr_state] = h[curr_state];
		  }
		else
		  {
		    if (ok)
		      {
			bdd last_ = iter_->current_accepting_conditions();
			bdd prop_ = iter_->current_condition();
			tmp_lst.push_back(state_proposition(curr_state, prop_));
			tmp_acc = last_ | step_.second;
			bdd curr_in = tmp_acc & to_accept;
			bdd best_in = best_acc & to_accept;
			if (curr_in == best_in)
			  {
			    if (tmp_lst.size() < best_lst.size())
			      {
				cycle_path tmp(tmp_lst);
				best_lst = tmp;
			      }
			  }
			else
			  {
			    if (bddtrue == (best_in >> curr_in))
			      {
				cycle_path tmp(tmp_lst);
				best_lst = tmp;
				best_acc = tmp_acc;
			      }
			  }
		      }
		    else
		      {
			bdd last_ = iter_->current_accepting_conditions();
			bdd prop_ = iter_->current_condition();
		        tmp_acc = last_ | step_.second;
			tmp_lst.push_back(state_proposition(curr_state,
							    prop_));
			cycle_path tmp(tmp_lst);
			best_lst = tmp;
			best_acc = tmp_acc;
			ok = true;
		      }
		  }
	      }
	    iter_->next();
	  }
      }
    for (cycle_path::iterator it = best_lst.begin();
	 it != best_lst.end(); ++it)
      period.push_back(*it);

    if (best_acc != to_accept)
      {
	bdd rec_to_acc = to_accept - best_acc;
	accepting_path(comp_path, period.back().first, rec_to_acc);
      }
    else
      {
	if (!period.empty())
	  {
	    /// The path contains all accepting conditions. Then we
	    ///complete the cycle in this SCC by calling complete_cycle.
	    complete_cycle(comp_path, period.back().first, suffix.back());
	  }
      }
  }
}
