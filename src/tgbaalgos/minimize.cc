// Copyright (C) 2010 Laboratoire de Recherche et Développement
// de l'Epita (LRDE).
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

#include <queue>
#include "minimize.hh"
#include "ltlast/allnodes.hh"
#include "misc/hash.hh"
#include "tgbaalgos/powerset.hh"

namespace spot
{
  typedef Sgi::hash_set<const state*,
                        state_ptr_hash, state_ptr_equal> hash_set;
  typedef Sgi::hash_map<const state*, unsigned,
                        state_ptr_hash, state_ptr_equal> hash_map;

  // Given an automaton a, find all states that are not in "final" and add
  // them to the set "non_final".
  void init_sets(const tgba_explicit* a,
                 hash_set& final,
                 hash_set& non_final)
  {
    hash_set seen;
    std::queue<const state*> tovisit;
    // Perform breadth-first traversal.
    const state* init = a->get_init_state();
    tovisit.push(init);
    seen.insert(init);
    while (!tovisit.empty())
    {
      const state* src = tovisit.front();
      tovisit.pop();
      // Is the state final ?
      if (final.find(src) == final.end())
        // No, add it to the set non_final
        non_final.insert(src);
      tgba_succ_iterator* sit = a->succ_iter(src);
      for (sit->first(); !sit->done(); sit->next())
      {
        const state* dst = sit->current_state();
        // Is it a new state ?
        if (seen.find(dst) == seen.end())
        {
          // Register the successor for later processing.
          tovisit.push(dst);
          seen.insert(dst);
        }
        else
          delete dst;
      }
      delete sit;
    }
  }

  // From the base automaton and the list of sets, build the minimal
  // resulting automaton
  tgba_explicit_number* build_result(const tgba* a,
                                     std::list<hash_set*>& sets,
                                     hash_set* final)
  {
    // For each set, create a state in the resulting automaton.
    // For a state s, state_num[s] is the number of the state in the minimal
    // automaton.
    hash_map state_num;
    std::list<hash_set*>::iterator sit;
    unsigned num = 0;
    for (sit = sets.begin(); sit != sets.end(); ++sit)
    {
      hash_set::iterator hit;
      hash_set* h = *sit;
      for (hit = h->begin(); hit != h->end(); ++hit)
        state_num[*hit] = num;
      ++num;
    }
    typedef tgba_explicit_number::transition trs;
    tgba_explicit_number* res = new tgba_explicit_number(a->get_dict());
    // For each transition in the initial automaton, add the corresponding
    // transition in res.
    if (!final->empty())
      res->declare_acceptance_condition(ltl::constant::true_instance());
    for (sit = sets.begin(); sit != sets.end(); ++sit)
    {
      hash_set::iterator hit;
      hash_set* h = *sit;
      for (hit = h->begin(); hit != h->end(); ++hit)
      {
        const state* src = *hit;
        unsigned src_num = state_num[src];
        tgba_succ_iterator* succit = a->succ_iter(src);
        bool accepting = (final->find(src) != final->end());
        for (succit->first(); !succit->done(); succit->next())
        {
          const state* dst = succit->current_state();
          unsigned dst_num = state_num[dst];
          delete dst;
          trs* t = res->create_transition(src_num, dst_num);
          res->add_conditions(t, succit->current_condition());
          if (accepting)
            res->add_acceptance_condition(t, ltl::constant::true_instance());
        }
        delete succit;
      }
    }
    res->merge_transitions();
    const state* init_state = a->get_init_state();
    unsigned init_num = state_num[init_state];
    delete init_state;
    res->set_init_state(init_num);
    return res;
  }

  tgba_explicit* minimize(const tgba* a)
  {
    // The list of accepting states of a.
    std::list<const state*> acc_list;
    std::queue<hash_set*> todo;
    // The list of equivalent states.
    std::list<hash_set*> done;
    tgba_explicit* det_a = tgba_powerset(a, &acc_list);
    hash_set* final = new hash_set;
    hash_set* non_final = new hash_set;
    hash_map state_set_map;
    bdd_dict* dict = det_a->get_dict();
    std::list<const state*>::iterator li;
    for (li = acc_list.begin(); li != acc_list.end(); ++li)
      final->insert(*li);

    init_sets(det_a, *final, *non_final);
    // Size of det_a
    unsigned size = final->size() + non_final->size();
    // Use bdd variables to number sets.  set_num is the first variable
    // available.
    unsigned set_num = dict->register_anonymous_variables(size, det_a);

    std::set<int> free_var;
    for (unsigned i = set_num; i < set_num + size; ++i)
      free_var.insert(i);
    std::map<int, int> used_var;

    if (!final->empty())
      {
	unsigned s = final->size();
	used_var[set_num] = s;
	free_var.erase(set_num);
	if (s > 1)
	  todo.push(final);
	else
	  done.push_back(final);
	for (hash_set::const_iterator i = final->begin();
	     i != final->end(); ++i)
	  state_set_map[*i] = set_num;
      }
    if (!non_final->empty())
      {
	unsigned s = non_final->size();
	unsigned num = set_num + 1;
	used_var[num] = s;
	free_var.erase(num);
	if (s > 1)
	  todo.push(non_final);
	else
	  done.push_back(non_final);
	for (hash_set::const_iterator i = non_final->begin();
	     i != non_final->end(); ++i)
	  state_set_map[*i] = num;
      }

    hash_set* final_copy = new hash_set(*final);
    // A bdd_states_map is a list of formulae (in a BDD form) associated with a
    // destination set of states.
    typedef std::list<std::pair<bdd, hash_set*> > bdd_states_map;
    // While we have unprocessed sets.
    while (!todo.empty())
    {
      // Get a set to process.
      hash_set* cur = todo.front();
      todo.pop();
      hash_set::iterator hi;
      bdd_states_map bdd_map;
      for (hi = cur->begin(); hi != cur->end(); ++hi)
      {
        const state* src = *hi;
        bdd f = bddfalse;
        tgba_succ_iterator* si = a->succ_iter(src);
        for (si->first(); !si->done(); si->next())
        {
          const state* dst = si->current_state();
          unsigned dst_set = state_set_map[dst];
          delete dst;
          f |= (bdd_ithvar(dst_set) & si->current_condition());
        }
        delete si;
        bdd_states_map::iterator bsi;
        // Have we already seen this formula ?
        for (bsi = bdd_map.begin(); bsi != bdd_map.end() && bsi->first != f;
             ++bsi)
          continue;
        if (bsi == bdd_map.end())
        {
          // No, create a new set.
          hash_set* new_set = new hash_set;
          new_set->insert(src);
          bdd_map.push_back(std::make_pair<bdd, hash_set*>(f, new_set));
        }
        else
        {
          // Yes, add the current state to the set.
          hash_set* set = bsi->second;
          set->insert(src);
        }
      }
      bdd_states_map::iterator bsi = bdd_map.begin();
      // The set is minimal.
      if (bdd_map.size() == 1)
        done.push_back(bsi->second);
      else
      {
        for (; bsi != bdd_map.end(); ++bsi)
        {
          hash_set* set = bsi->second;
          // Free the number associated to these states.
	  unsigned num = state_set_map[*set->begin()];
	  assert(used_var.find(num) != used_var.end());
	  unsigned left = (used_var[num] -= set->size());
	  // Make sure LEFT does not become negative (hence bigger
	  // than SIZE when read as unsigned)
	  assert(left < size);
	  if (left == 0)
	    {
	      used_var.erase(num);
	      free_var.insert(num);
	    }
	  // Pick a free number
	  assert(!free_var.empty());
	  num = *free_var.begin();
	  free_var.erase(free_var.begin());
	  used_var[num] = set->size();
          for (hash_set::iterator hit = set->begin(); hit != set->end(); ++hit)
	    state_set_map[*hit] = num;
          // Trivial sets can't be splitted any further.
          if (set->size() == 1)
            done.push_back(set);
          else
            todo.push(set);
        }
      }
      delete cur;
    }

    // Build the result.
    tgba_explicit_number* res = build_result(det_a, done, final_copy);

    // Free all the allocated memory.
    delete final_copy;
    hash_map::iterator hit;
    for (hit = state_set_map.begin(); hit != state_set_map.end(); ++hit)
      delete hit->first;
    std::list<hash_set*>::iterator it;
    for (it = done.begin(); it != done.end(); ++it)
      delete *it;
    delete det_a;

    return res;
  }
}
