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
    std::stack<pair_state_iter> todo;
    std::stack<bdd> arc_accepting;
    int nbstate = 1;
    state* init = aut_->get_init_state();
    h[init] = 1;
    root_component.push(connected_component(1));
    arc_accepting.push(bddfalse);
    tgba_succ_iterator* iter_ = aut_->succ_iter(init);
    iter_->first();
    todo.push(pair_state_iter(init, iter_));
    while (!todo.empty())
      {
	pair_state_iter step = todo.top();
	if (step.second->done())
	  {
	    todo.pop();
	    assert(!root_component.empty());
	    connected_component comp_tmp = root_component.top();
	    root_component.pop();
	    hash_type::iterator i_0 = h.find(step.first);
	    assert(i_0 != h.end());
	    if (comp_tmp.index == i_0->second)
	      {
		// The current node is a root of a Strong Connected Component.
		remove_component(step.first);
		assert(!arc_accepting.empty());
		arc_accepting.pop();
	      }
	    else
	      {
		root_component.push(comp_tmp);
	      }
	    assert(root_component.size() == arc_accepting.size());
	  }
	else
	  {
	    iter_ = step.second;
	    state* current_state = iter_->current_state();
	    bdd current_accepting = iter_->current_accepting_conditions();
	    hash_type::iterator i = h.find(current_state);
	    iter_->next();
	    if (i == h.end())
	      {
		// New node.
		nbstate = nbstate + 1;
		assert(nbstate != 0);
		h[current_state] = nbstate;
		root_component.push(connected_component(nbstate));
		arc_accepting.push(current_accepting);
		tgba_succ_iterator* iter2 = aut_->succ_iter(current_state);
		iter2->first();
		todo.push(pair_state_iter(current_state, iter2));
	      }
	    else if (i->second != -1)
	      {
		// A node with order != -1 (a seen node not removed).
		assert(!root_component.empty());
		connected_component comp = root_component.top();
		root_component.pop();
		bdd new_condition = current_accepting;
		int current_index = i->second;
		while (comp.index > current_index)
		  {
		    // root_component and arc_accepting are popped
		    // until the head of root_component is less or
		    // equal to the order of the current state.
		    assert(!root_component.empty());
		    comp = root_component.top();
		    root_component.pop();
		    new_condition |= comp.condition;
		    assert(!arc_accepting.empty());
		    bdd arc_acc = arc_accepting.top();
		    arc_accepting.pop();
		    new_condition |= arc_acc;
		  }
		comp.condition |= new_condition;
		if (aut_->all_accepting_conditions() == comp.condition)
		  {
		    // A failure SCC was found, the automata is not empty.
		    root_component.push(comp);
      		    return false;
		  }
		root_component.push(comp);
		assert(root_component.size() == arc_accepting.size());
	      }
	  }
      }
    // The automata is empty.
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
	    os << restrict->format_state(aut_->project_state(*i_se, restrict))
	       << std::endl;
	  }
	else
	  {
	    os << aut_->format_state((*i_se)) << std::endl;
	  }
      }
    os << "Cycle:" <<std::endl;
    for (cycle_path::const_iterator it = period.begin();
	 it != period.end(); ++it)
      {
	os << "  ";
	if (restrict)
	  {
	    os << "    | " << bdd_format_set(d, it->second) <<std::endl ;
	    os << restrict->format_state(aut_->project_state(it->first,
							     restrict))
	       << std::endl;
	  }
	else
	  {
	    os << "    | " << bdd_format_set(d, it->second) <<std::endl ;
	    os << aut_->format_state(it->first) << std::endl;
	  }
      }
    return os;
  }

  void
  emptiness_check::counter_example()
  {
    std::deque <pair_state_iter> todo_trace;
    typedef std::map<const state*, const state*,
                     state_ptr_less_than> path_state;
    path_state path_map;

    assert(!root_component.empty());

    int comp_size = root_component.size();
    typedef std::vector<connected_component> vec_compo;
    vec_compo vec_component(comp_size);
    std::vector<state_sequence> vec_sequence(comp_size);
    state_sequence seq;
    state_sequence tmp_lst;
    state_sequence best_lst;
    bdd tmp_acc = bddfalse;
    std::stack<pair_state_iter> todo_accept;

    for (int j = comp_size - 1; j >= 0; j--)
      {
	vec_component[j] = root_component.top();
	root_component.pop();
      }

    int q_index;
    int tmp_int = 0;
    // Fill the SCC in the stack root_component.
    for (hash_type::iterator iter_map = h.begin();
	 iter_map != h.end(); ++iter_map)
      {
	q_index = iter_map->second;
	tmp_int = 0;
	if (q_index > 0)
	  {
	    while ((tmp_int < comp_size)
		   && (vec_component[tmp_int].index <= q_index))
	      tmp_int = tmp_int+1;
	    if (tmp_int < comp_size)
	      vec_component[tmp_int - 1].state_set.insert(iter_map->first);
	    else
	      vec_component[comp_size - 1].state_set.insert(iter_map->first);
	  }
      }

    state* start_state = aut_->get_init_state();
    if (comp_size != 1)
      {
	tgba_succ_iterator* i = aut_->succ_iter(start_state);
	todo_trace.push_back(pair_state_iter(start_state, i));

	for (int k = 0; k < comp_size - 1; ++k)
	  {
	    // We build a path trought all SCC in the stack: a
	    // possible prefix for a counter example.
	    while (!todo_trace.empty())
	      {
		pair_state_iter started_from = todo_trace.front();
		todo_trace.pop_front();

		for (started_from.second->first();
		     !started_from.second->done();
		     started_from.second->next())
		  {
		    const state* curr_state =
		      started_from.second->current_state();
		    if (vec_component[k+1].has_state(curr_state))
		      {
			const state* curr_father = started_from.first;
			seq.push_front(curr_state);
			seq.push_front(curr_father);
			hash_type::iterator i_2 = h.find(curr_father);
			assert(i_2 != h.end());
			while (vec_component[k].index < i_2->second)
			  {
			    assert(i_2->second != 1);
			    assert(path_map.find(curr_father)
				   != path_map.end());
			    const state* f = path_map[curr_father];
			    seq.push_front(f);
			    curr_father = f;
			    i_2 = h.find(curr_father);
			    assert(i_2 != h.end());
			  }
			vec_sequence[k] = seq;
			seq.clear();
			todo_trace.clear();
			break;
		      }
		    else
		      {
			if (vec_component[k].has_state(curr_state))
			  {
			    path_state::iterator i_path =
			      path_map.find(curr_state);
			    hash_type::iterator i_seen = h.find(curr_state);

			    if (i_seen != h.end()
				&& i_seen->second > 0
				&& i_path == path_map.end())
			      {
				todo_trace.
				  push_back(pair_state_iter(curr_state,
							    aut_->succ_iter(curr_state)));
				path_map[curr_state] = started_from.first;
			      }
			  }
		      }
		  }
	      }
	    todo_trace.
	      push_back(pair_state_iter(vec_sequence[k].back(),
					aut_->succ_iter(vec_sequence[k].back())));
	  }
      }
    else
      {
	suffix.push_front(start_state);
      }
    for (int n_ = 0; n_ < comp_size - 1; ++n_)
      for (state_sequence::iterator it = vec_sequence[n_].begin();
	   it != vec_sequence[n_].end(); ++it)
	suffix.push_back(*it);
    suffix.unique();
    accepting_path(vec_component[comp_size - 1], suffix.back(),
		   vec_component[comp_size - 1].condition);
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
