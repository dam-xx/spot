#include "emptinesscheck.hh"
#include "tgba/tgba.hh"
#include "tgba/state.hh"
#include "tgba/bddprint.hh"
#include "tgba/tgbabddfactory.hh"
#include "tgba/succiterconcrete.hh"
#include "tgba/tgbabddconcrete.hh"
#include "bdd.h"
#include <map>
#include <list>
#include <sstream>
#include <stack>
#include <queue>
#include <stdio.h>
#include <vector>
#include <set>

namespace spot
{
  connected_component::connected_component()
  {
	index = 0;
	condition = bddfalse;
	transition_acc = -1;
	nb_transition = 0;
	nb_state = 1;
	not_null = false;
  }

    connected_component::connected_component(int i, bdd a)
    {
      index = i;
      condition = a;
      transition_acc = -1;
      nb_transition = 0;
      nb_state = 1;
      not_null = false;
    }

  connected_component::~connected_component()
  {
  }

  std::string
  connected_component::to_string_component()
  {
    return "+ index + condition + nbTransition + transitionCondition + notNull +";
  }

  bool
  connected_component::isAccepted(tgba* aut)
  {
    return aut->all_accepting_conditions() == condition;
  }

  void
    emptiness_check::remove_component(const tgba& aut, seen& state_map, const spot::state* start_delete)
    {
    /// \brief Remove all the nodes accessible from the given node start_delete.
    ///
    /// The removed graphe is the subgraphe containing nodes store
    ///intable state_map with order -1.

    std::stack<spot::tgba_succ_iterator*> to_remove;
    state_map[start_delete] = -1;
    tgba_succ_iterator* iter_delete = aut.succ_iter(start_delete);
    iter_delete->first();
    to_remove.push(iter_delete);
     while (!to_remove.empty())
      {
	tgba_succ_iterator* succ_delete = to_remove.top();
	to_remove.pop();
	if (!succ_delete->done())
	  {
	    to_remove.push(succ_delete);
	    state* curr_state = succ_delete->current_state();
	    succ_delete->next();
	    if (state_map[curr_state] != -1)
	      {
		state_map[curr_state] = -1;
		tgba_succ_iterator* succ_delete2 = aut.succ_iter(curr_state);
		succ_delete2->first();
		to_remove.push(succ_delete2);
	      }
	  }
      }
     }

  emptiness_check::~emptiness_check()
    {
    }
emptiness_check::emptiness_check()
    {
    }

  bool
  emptiness_check::tgba_emptiness_check(const spot::tgba* aut_check)
  {
    /// \brief On-the-fly emptiness check.
    ///
    /// The algorithm used here is adapted from Jean-Michel Couvreur's
    /// Probataf tool.
    int nbstate = 1;
    state* init = aut_check->get_init_state();
    seen_state_num[init] = 1;
    root_component.push(spot::connected_component(1,bddfalse));
     arc_accepting.push(bddfalse);
    tgba_succ_iterator* iter_ = aut_check->succ_iter(init);
    iter_->first();
    todo.push(pair_state_iter(init, iter_ ));
     while (!todo.empty())
      {
	pair_state_iter step = todo.top();
	if ((step.second)->done())
	  {
	    todo.pop();
	    assert(!root_component.empty());
	    connected_component comp_tmp = root_component.top();
	    root_component.pop();
	     seen::iterator i_0 = seen_state_num.find(step.first);
	     assert(i_0 != seen_state_num.end());
	    if (comp_tmp.index == seen_state_num[step.first])
	      {
		/// The current node is a root of a Strong Connected Component.
		spot::emptiness_check::remove_component(*aut_check, seen_state_num, step.first);
		assert(!arc_accepting.empty());
		arc_accepting.pop();
		assert(root_component.size() == arc_accepting.size());
	      }
	    else
	      {
		root_component.push(comp_tmp);
		assert(root_component.size() == arc_accepting.size());
	      }
	  }
	else
	  {
	    iter_ = step.second;
	    state* current_state = iter_->current_state();
	    bdd current_accepting = iter_->current_accepting_conditions();
	    seen::iterator i = seen_state_num.find(current_state);
	    iter_->next();
	    if (i == seen_state_num.end())
	      {
		/// New node.
		nbstate = nbstate + 1;
		 assert(nbstate != 0);
		seen_state_num[current_state] = nbstate;
		root_component.push(connected_component(nbstate, bddfalse));
		arc_accepting.push(current_accepting);
		tgba_succ_iterator* iter2 = aut_check->succ_iter(current_state);
		iter2->first();
		todo.push(pair_state_iter(current_state, iter2 ));
	      }
	    else if (seen_state_num[current_state] != -1)
	      {
		/// A node with order != -1 (a seen node not removed).
		assert(!root_component.empty());
		connected_component comp = root_component.top();
		root_component.pop();
		bdd new_condition = bddfalse;
		new_condition = bdd_apply(new_condition, current_accepting, bddop_or);
		new_condition = bdd_apply(new_condition, comp.condition, bddop_or);
		int current_index = seen_state_num[current_state];
		while (comp.index > current_index)
		  {
		    /// root_component and arc_accepting are popped
		    /// until the head of root_component is less or
		    /// equal to the order of the current state.
		    assert(!root_component.empty());
		    comp = root_component.top();
		    root_component.pop();
		    new_condition = bdd_apply(new_condition,comp.condition, bddop_or);
		    assert(!arc_accepting.empty());
		    bdd arc_acc = arc_accepting.top();
		    arc_accepting.pop();
		    new_condition = bdd_apply(new_condition, arc_acc, bddop_or);
		  }
		comp.condition = bdd_apply(comp.condition, new_condition, bddop_or);
		if (aut_check->all_accepting_conditions() == comp.condition)
		  {
		    /// A failure SCC is find, the automata is not empty.
		    spot::bdd_print_dot(std::cout, aut_check->get_dict(),comp.condition);
		    root_component.push(comp);
		    return false;
		  }
		root_component.push(comp);
		assert(root_component.size() == arc_accepting.size());
	      }
	  }
      }
    spot::bdd_print_dot(std::cout, aut_check->get_dict(),aut_check->all_accepting_conditions());
    /// The automata is empty.
    return true;
  }


  void
    emptiness_check::counter_example(const spot::tgba* aut_counter)
    {
      /// \brief Build a possible prefixe and period for a counter example.
      bool emptiness = tgba_emptiness_check(aut_counter);
      std::deque <pair_state_iter> todo_trace;
      typedef std::map <const spot::state*, const spot::state*, spot::state_ptr_less_than> path_state;
      path_state path_map;

      if (!emptiness){
	int comp_size = root_component.size();
	std::cout << "*****COUNTER-EXAMPLE*****" << comp_size << std::endl;
	typedef std::vector<connected_component> vec_compo;
	vec_compo vec_component;
	vec_component.resize(comp_size);
       	vec_sequence.resize(comp_size);
	state_sequence seq;
	state_sequence tmp_lst;
	state_sequence best_lst;
	bdd tmp_acc = bddfalse;
 std::stack <pair_state_iter> todo_accept;

	for (int j = comp_size -1; j >= 0; j--)
	  {
	    vec_component[j] = root_component.top();
spot::bdd_print_dot(std::cout, aut_counter->get_dict(),root_component.top().condition);
	    root_component.pop();
	  }

	for (int p = 0; p < comp_size; p++)
	  {
	    std::cout << "*****" <<  vec_component[p].index << "*****" << std::endl;
	  }
	int q_index;
	int tmp_int = 0;
	/// Fill the SCC in the stack root_component.
	for (seen::iterator iter_map = seen_state_num.begin(); iter_map != seen_state_num.end(); iter_map++)
	  {
	    q_index = (*iter_map).second;
	    tmp_int = 0;
	    if (q_index > 0)
	      {
		while ((tmp_int < comp_size) && (vec_component[tmp_int].index <= q_index))
		  {
		    tmp_int = tmp_int+1;
		  }
		if (tmp_int < comp_size)
		  {
		    vec_component[tmp_int-1].state_set.insert((*iter_map).first);
		  }
		else
		  {
		    vec_component[comp_size-1].state_set.insert((*iter_map).first);
		  }
	      }
	  }

	for (int m = 0; m < comp_size; m++)
	  {
	    std::cout << "*****CONNECTED COMPONENT :" <<vec_component[m].index << "*****" << std::endl;
	    for (connected_component::set_of_state::iterator i_seq = vec_component[m].state_set.begin(); i_seq != vec_component[m].state_set.end(); i_seq++)
	      {
		std::cout << "*****STATE :" << aut_counter->format_state(*i_seq) << "*****" << std::endl;
		seen::iterator i_1 = seen_state_num.find((*i_seq));
		assert(i_1 != seen_state_num.end());
		std::cout << "***** NUM :" << seen_state_num[*i_seq] << "*****" << std::endl;
	      }
	    std::cout << "_________________________________" << std::endl;
	  }

	state* start_state = aut_counter->get_init_state();
	if (comp_size != 1)
	  {
	todo_trace.push_back(pair_state_iter(start_state, aut_counter->succ_iter(start_state)));

        for (int k = 0; k < comp_size-1; k++)
	  {
	    /// We build a path trought all SCC in the stack : a
	    ///possible prefixe for a counter example.
	    while (!todo_trace.empty())
	    {
	      pair_state_iter started_from = todo_trace.front();
	      todo_trace.pop_front();
	      (started_from.second)->first();

	      for ((started_from.second)->first(); !started_from.second->done(); started_from.second->next())
		{
		  const state* curr_state =(started_from.second)->current_state();
connected_component::set_of_state::iterator iter_set = vec_component[k+1].state_set.find(curr_state);
		  if (iter_set != vec_component[k+1].state_set.end())
		      {
			const state* curr_father = started_from.first;
			seq.push_front(*iter_set);
			seq.push_front(curr_father);
			 seen::iterator i_2 = seen_state_num.find(curr_father);
			 assert(i_2 != seen_state_num.end());
			while ((vec_component[k].index < seen_state_num[curr_father]) && (seen_state_num[curr_father] != 1))
			{
			  seq.push_front(path_map[curr_father]);
			  curr_father = path_map[curr_father];
			  seen::iterator i_3 = seen_state_num.find(curr_father);
			 assert(i_3 != seen_state_num.end());
			}
			vec_sequence[k] = seq;
			seq.clear();
			todo_trace.clear();
			break;
		      }

		    else
		      {
			connected_component::set_of_state::iterator i_s = vec_component[k].state_set.find(curr_state);
			if (i_s != vec_component[k].state_set.end())
			  {
			    path_state::iterator i_path = path_map.find(curr_state);
			    seen::iterator i_seen = seen_state_num.find(curr_state);

			    if (i_seen != seen_state_num.end() && seen_state_num[curr_state] > 0 && i_path == path_map.end())
			      {
				todo_trace.push_back(pair_state_iter(curr_state, aut_counter->succ_iter(curr_state)));
				path_map[curr_state] = started_from.first;
			      }
			  }
		      }
		}
	    }
     todo_trace.push_back(pair_state_iter(vec_sequence[k].back(), aut_counter->succ_iter(vec_sequence[k].back())));
	  }
	  }
	else
	  {
	    seq_counter.push_front(start_state);
	  }
	//	vec_sequence[0].push_front(start_state);
  for (int n_ = 0; n_ < comp_size-1; n_++)
    {
      for (state_sequence::iterator it = vec_sequence[n_].begin(); it != vec_sequence[n_].end(); it++)
	    {
	      seq_counter.push_back(*it);
	    }
      //  seq_counter.splice(seq_counter.end(), vec_sequence[n_]);
    }
  seq_counter.unique();
  for (state_sequence::iterator i_se = seq_counter.begin(); i_se != seq_counter.end(); i_se++)
	    {
	      std::cout << "*****STATE :" << aut_counter->format_state(*i_se) << "*****" << std::endl;
	      std::cout << "***** NUM :" << seen_state_num[(*i_se)] << "*****" << std::endl;
	    }

  std::cout << "ACCEPTING PATH BEGIN " << std::endl;
  /// Call accepting_path to build the period of the counter example.
  emptiness_check::accepting_path(aut_counter, vec_component[comp_size-1], seq_counter.back(),vec_component[comp_size-1].condition);
std::cout << "ACCEPTING PATH END " << std::endl;

      }
      else
	{
	  std::cout << "Nothings in vec_sequence " << std::endl;
	  std::cout << "******************************************" << std::endl;
	  std::cout << "*****SEEN STATE NUM :" << "*****" << std::endl;
	  for (seen::iterator i_sd = seen_state_num.begin(); i_sd != seen_state_num.end(); i_sd++)
	    {
	      std::cout << "*****STATE :" << aut_counter->format_state((*i_sd).first) << "*****" << std::endl;
	      std::cout << "***** NUM :" << seen_state_num[(*i_sd).first] << "*****" << std::endl;
	      }
	  std::cout << "_________________________________" << std::endl;
	}
    }

 void
   emptiness_check::complete_cycle(const spot::tgba* aut_counter, const connected_component& comp_path, const state* from_state,const state* to_state)
   {
     /// \brief complete the path build by accepting_path to get the
     ///period (cycle).
     if (seen_state_num[from_state] != seen_state_num[to_state])
       {
	 std::cout << "COMPLETE CYCLE BEGIN" << std::endl;
	 std::map <const spot::state*, state_proposition, spot::state_ptr_less_than> complete_map;
	 std::deque <pair_state_iter> todo_complete;
	 spot::tgba_succ_iterator* ite = aut_counter->succ_iter(from_state);
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
		 const state* curr_state = (started_.second)->current_state();
		 connected_component::set_of_state::iterator i_set = comp_path.state_set.find(curr_state);
		 if (i_set != comp_path.state_set.end())
		   {
		     if (curr_state->compare(to_state) == 0)
		       {
			 const state* curr_father = started_.first;
			 bdd curr_condition = iter_s->current_condition();
			 tmp_comp.push_front(state_proposition(curr_state, curr_condition));
			 // tmp_comp.push_front(state_proposition(curr_father, complete_map[curr_father].second));
			 while (curr_father->compare(from_state) != 0)
			   {
			     //emptiness_check::periode.push_front(state_proposition(curr_father, complete_map[curr_father].second));
			     tmp_comp.push_front(state_proposition(curr_father, complete_map[curr_father].second));
			     curr_father = complete_map[curr_father].first;
			   }
			 emptiness_check::periode.splice(periode.end(), tmp_comp);
			 todo_complete.clear();
			 break;
		       }
		     else
		       {
			 todo_complete.push_back(pair_state_iter(curr_state, aut_counter->succ_iter(curr_state)));
			 complete_map[curr_state] = state_proposition(started_.first, iter_s->current_condition());
		       }
		   }
	       }
	   }
	 //     emptiness_check::periode = state_prop;
       }
     std::cout << "END COMPLETE CYCLE " << std::endl;
   }


void
  emptiness_check::accepting_path(const spot::tgba* aut_counter, const connected_component& comp_path, const spot::state* start_path, bdd to_accept)
  {
    /// \Brief build recursively a path in the accepting SCC to get
    ///all accepting conditions. This path is the first part of the
    ///period.
    seen seen_priority;
    std::stack<triplet> todo_path;
    tgba_succ_iterator* t_s_i = aut_counter->succ_iter(start_path);
    t_s_i->first();
    todo_path.push(triplet(pair_state_iter(start_path,t_s_i), bddfalse));
    bdd tmp_acc = bddfalse;
    bdd best_acc = bddfalse;
    cycle_path tmp_lst;
    cycle_path best_lst;
    bool ok = false;
    seen_priority[start_path] = seen_state_num[start_path];
    for (seen::iterator i_ss = seen_priority.begin(); i_ss != seen_priority.end(); i_ss++)
      {
	std::cout << "*****STATE :" << aut_counter->format_state((*i_ss).first) << "*****" << std::endl;
	std::cout << "***** NUM :" << seen_priority[(*i_ss).first] << "*****" << std::endl;
      }
    while (!todo_path.empty())
      {
	triplet step_ = todo_path.top();
	tgba_succ_iterator* iter_ = (step_.first).second;
	std::cout << "WHILE BEGIN " << iter_ << std::endl;
	if (iter_->done())
	  {
	    std::cout << "IF BEGIN " << std::endl;
	    todo_path.pop();
	    seen_priority.erase((step_.first).first);
	  for (seen::iterator i_ss = seen_priority.begin(); i_ss != seen_priority.end(); i_ss++)
	 {
       std::cout << "*****STATE :" << aut_counter->format_state((*i_ss).first) << "*****" << std::endl;
	std::cout << "***** NUM :" << seen_priority[(*i_ss).first] << "*****" << std::endl;
	    }
	    //seen_priority.[(step_.first).first] = -2;
	    //delete(tmp_lst.back().first);
	    tmp_lst.pop_back();
	    std::cout << "IF END " << std::endl;
	  }
	else
	  {
	    std::cout << "ELSE BEGIN " << std::endl;
	    state* curr_state = iter_->current_state();
	    connected_component::set_of_state::iterator it_set = comp_path.state_set.find(curr_state);
	    if (it_set != comp_path.state_set.end())
	      {
		std::cout << "IN COMPONENT " << std::endl;
		seen::iterator i = seen_priority.find(curr_state);
		if (i == seen_priority.end())
		  {
		 std::cout << "NOT IN MAP " << std::endl;
		 std::cout <<"STATE:" << aut_counter->format_state(curr_state) << "*****" << std::endl;
		 // spot::bdd_print_dot(std::cout, aut_counter->get_dict(),aut_counter->succ_iter(curr_state)->current_accepting_conditions());
		 tgba_succ_iterator* c_iter = aut_counter->succ_iter(curr_state);
		 bdd curr_bdd =  bdd_apply(iter_->current_accepting_conditions(), step_.second, bddop_or);
		 std::cout << "*****TODO PATH PUSH STATE : BEFORE" << aut_counter->format_state((todo_path.top().first).first) << "*****" << std::endl;
		 c_iter->first();
		 todo_path.push(triplet(pair_state_iter(curr_state, c_iter), curr_bdd));
std::cout << "*****TODO PATH PUSH STATE : AFTER" << aut_counter->format_state((todo_path.top().first).first) << "*****" << std::endl;
		    tmp_lst.push_back(state_proposition(curr_state, iter_->current_condition()));
		    seen_priority[curr_state] = seen_state_num[curr_state];
		  }
		else
		  {
		    std::cout << "IN MAP " << std::endl;
		    if (ok)
		      {
			std::cout << "NOT FIRST TIME " << std::endl;
			bdd last_ = iter_->current_accepting_conditions();
			bdd prop_ = iter_->current_condition();
			tmp_lst.push_back(state_proposition(curr_state, prop_));
			tmp_acc = bdd_apply(last_, step_.second, bddop_or);
//			tmp_lst.push_back(state_proposition(curr_state->clone(), last_));
			bdd curr_in = bdd_apply(tmp_acc, to_accept, bddop_and);
			bdd best_in = bdd_apply(best_acc, to_accept, bddop_and);
			if (curr_in == best_in)
			  {
			    if (tmp_lst.size() < best_lst.size())
			      {
				std::cout << "tmp_lst.size() < best_lst.size() " << std::endl;
				cycle_path tmp(tmp_lst);
				best_lst = tmp;
				spot::bdd_print_dot(std::cout, aut_counter->get_dict(),step_.second);
			      }
			  }
			else
			  {
			    if (bddtrue == bdd_apply(best_in, curr_in, bddop_imp))
			      {
			    cycle_path tmp(tmp_lst);
			    best_lst = tmp;
			    best_acc = tmp_acc;
			      }
			  }
		      }
		    else
		      {
			std::cout << "FIRST TIME " << std::endl;
			bdd last_ = iter_->current_accepting_conditions();
			bdd prop_ = iter_->current_condition();
		        tmp_acc = bdd_apply(last_, step_.second, bddop_or);
			tmp_lst.push_back(state_proposition(curr_state, prop_));
			cycle_path tmp(tmp_lst);
			best_lst = tmp;
			best_acc = tmp_acc;
			ok = true;
		      }
		  }
	      }
	    iter_->next();
	    std::cout << "ELSE END " << std::endl;
	  }
      }
    for (cycle_path::iterator it = best_lst.begin(); it != best_lst.end(); it++)
	    {
	      emptiness_check::periode.push_back(*it);
	    }
    //  emptiness_check::periode.splice(periode.end(), best_lst);
     std::cout <<  "POINTEUR:" << periode.back().first << "*STATE :" << aut_counter->format_state( periode.back().first) << "*****" << std::endl;
    if (best_acc != to_accept)
      {
	std::cout << "IN RECURSE " << std::endl;
	bdd rec_to_acc = bdd_apply(to_accept, !best_acc, bddop_and);
	std::cout << "BEST_ACC " << std::endl;
	spot::bdd_print_dot(std::cout, aut_counter->get_dict(),best_acc);
	std::cout << "TO_ACCEPT " << std::endl;
	spot::bdd_print_dot(std::cout, aut_counter->get_dict(),to_accept);
	std::cout << "TO_ACCEPT - BEST_ACC " << std::endl;
	spot::bdd_print_dot(std::cout, aut_counter->get_dict(),rec_to_acc);
	emptiness_check::accepting_path(aut_counter, comp_path, periode.back().first, rec_to_acc);
	std::cout << "IN RECURSE " << std::endl;
      }
    else
      {
	bdd rec_to_acc = bdd_apply(to_accept, !best_acc, bddop_and);
	std::cout << "TO_ACCEPT - BEST_ACC " << std::endl;
	spot::bdd_print_dot(std::cout, aut_counter->get_dict(),rec_to_acc);
	if (!periode.empty())
	  {

	    std::cout << "IN COMPLETE CYCLE " << std::endl;
	    /// The paht contains all accepting conditions. Then we
	    ///complete the cycle in this SCC by calling complete_cycle.
	complete_cycle(aut_counter, comp_path, periode.back().first, seq_counter.back());
	for (cycle_path::iterator it = periode.begin(); it != periode.end(); it++)
	    {
	      std::cout << "*****STATE :" << aut_counter->format_state((*it).first) << "*****" << std::endl;
	      std::cout << "PROPOSITION TRANSITION :" << std::endl;
	      spot::bdd_print_dot(std::cout, aut_counter->get_dict(), (*it).second);
	    }
	  }
      }
  }
}
