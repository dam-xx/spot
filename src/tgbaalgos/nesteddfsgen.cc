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

#include "tgbaalgos/nesteddfsgen.hh"

namespace spot
{

  nesteddfsgen_search::nesteddfsgen_search(const tgba *a)
    : a(a)
  {
  }

  nesteddfsgen_search::~nesteddfsgen_search()
  {
    for (stack_type::iterator i = stack.begin();
	 i != stack.end(); ++i)
      {
	//delete i->first;
	delete i->second;
      }
    for (hash_type::iterator i = h.begin();
	 i != h.end();)
      {
	const state *s = i->first;
	++i;
	delete s;
      }
  }

  void
  nesteddfsgen_search::free_states()
  {
    for (std::list<const state*>::iterator i = states->begin();
	 i != states->end(); ++i)
      {
	delete *i;
      }
    delete states;
  }

  //ce::counter_example*
  /*
  bool
  nesteddfsgen_search::check()
  {
    hash_type::iterator hi, hi_next;
    tgba_succ_iterator *iter, *iter_temp;
    const state *init, *s, *s_next, *succ;
    bdd cond1, cond2;

    init = a->get_init_state();
    iter_temp = a->succ_iter(init);
    iter_temp->first();
    stack.push_front(state_iter_pair(init, iter_temp));

    while (!stack.empty())
      {
	s = stack.front().first;
	iter = stack.front().second;

	while (!iter->done())
	  {
	    succ = iter->current_state(); //
	    hi = h.find(succ);
	    if (hi != h.end() &&
		(hi->second.in_stack ||
		 hi->second.processed))
	      {
		delete succ;
		iter->next();
		continue;
	      }
	    iter_temp = a->succ_iter(succ);
	    iter_temp->first();
	    stack.push_front(state_iter_pair(succ, iter_temp));
	    if (hi != h.end())
	      hi->second.in_stack = 1;
	    else
	      {
		state_info si = { 1, 0, bddfalse };
		h[s] = si;
	      }
	    s = succ;
	    iter->next();
	    iter =  iter_temp;
	  }

	//delete iter;
	iter = a->succ_iter(s);
	hi = h.find(s);

	for (iter->first(); !iter->done(); iter->next())
	  {
	    s_next = iter->current_state(); //
	    hi_next = h.find(s_next);
	    if (hi != h.end())
	      cond1 = hi->second.cond;
	    else
	      cond1 = bddfalse;
	    if (hi_next != h.end())
	      cond2 = hi_next->second.cond;
	    else
	      cond2 = bddfalse;
	    if ((!(iter->current_acceptance_conditions() | cond1) |
		 cond2) != bddtrue)
	      markConditions(s_next,
			     iter->current_acceptance_conditions()
			     | cond1);
	    //else
	    //delete s_next;
	  }
	delete iter;

	if (hi != h.end() &&
	    hi->second.cond == a->all_acceptance_conditions())
	  {
	    std::cout << "Counter example found" << std::endl;
	    return 1;
	  }

	//free_states();

	if (hi == h.end())
	  {
	    state_info si = { 0, 1, bddfalse };
	    h[s] = si;
	  }
	else
	  {
	    hi->second.processed = 1;
	    //delete s;
	  }

	hi = h.find(stack.front().first);
	if (hi == h.end())
	  delete stack.front().first;
	delete stack.front().second;
	//iter = stack.front().second;
	//delete iter;
	stack.pop_front();

      }

    std::cout << "Automate is inf-empty" << std::endl;
    return 0;
  }
  */

  bool
  nesteddfsgen_search::check()
  {
    hash_type::iterator hi, hi_next;
    tgba_succ_iterator *iter, *iter_temp;
    const state *init, *s, *s_next, *succ;
    bdd cond1, cond2;

    init = a->get_init_state();
    iter_temp = a->succ_iter(init);
    iter_temp->first();
    stack.push_front(state_iter_pair(init, iter_temp));
    state_info si = { 1, 0, bddfalse };
    h[init] = si;

    while (!stack.empty())
      {
      recurse:
	s = stack.front().first;
	iter = stack.front().second;

	while (!iter->done())
	  {
	    //std::cout << "while (!iter->done())" << std::endl;
	    succ = iter->current_state();

	    hi = h.find(succ);
	    if (hi != h.end() &&
		(hi->second.in_stack ||
		 hi->second.processed))
	      {
		delete succ;
		iter->next();
		continue;
	      }

	    iter->next();

	    iter = a->succ_iter(succ);
	    iter->first();
	    stack.push_front(state_iter_pair(succ, iter));

	    state_info si = { 1, 0, bddfalse };
	    h[succ] = si;
	    goto recurse;
	  }

	iter = a->succ_iter(s);
	hi = h.find(s);

	for (iter->first(); !iter->done(); iter->next())
	  {
	    //std::cout << "for (iter->first(); !iter->done(); iter->next())"
	    //<< std::endl;
	    s_next = iter->current_state(); //
	    hi_next = h.find(s_next);
	    if (hi != h.end())
	      cond1 = hi->second.cond;
	    else
	      cond1 = bddfalse;
	    if (hi_next != h.end())
	      cond2 = hi_next->second.cond;
	    else
	      cond2 = bddfalse;
	    if ((!(iter->current_acceptance_conditions() | cond1) |
		 cond2) != bddtrue)
	      markConditions(s_next,
			     iter->current_acceptance_conditions()
			     | cond1);
	    else
	      delete s_next;
	  }
	delete iter;

	if (hi != h.end() &&
	    hi->second.cond == a->all_acceptance_conditions())
	  {
	    std::cout << "Counter example found" << std::endl;
	    return 1;
	  }

	if (hi == h.end())
	  {
	    state_info si = { 0, 1, bddfalse };
	    h[s] = si;
	  }
	else
	  {
	    hi->second.processed = 1;
	  }

	hi = h.find(stack.front().first);
	if (hi == h.end())
	  delete stack.front().first;
	delete stack.front().second;
	stack.pop_front();

      }

    std::cout << "Automate is inf-empty" << std::endl;
    return 0;
  }

  void
  nesteddfsgen_search::markConditions(const state* s, bdd conditions)
  {
    //std::cout << "markConditions" << std::endl;

     states = new std::list<const state*>;
     hash_type::iterator hi;
     tgba_succ_iterator* iter;
     const state *q_next, *q;
     bdd cond;

     states->push_front(s);
     do
       {
	 q = states->front();
	 states->pop_front();
	 hi = h.find(q);
	 if (hi != h.end())
	   {
	     delete q;
	     q = hi->first;
	     hi->second.cond |= conditions;
	   }
	 else
	   {
	     state_info si = { 0, 0, conditions };
	     h[q] = si;
	   }
	 iter = a->succ_iter(q);
	 for (iter->first(); !iter->done(); iter->next())
	   {
	     q_next = iter->current_state();
	     hi = h.find(q_next);
	     if (hi != h.end())
	       cond = hi->second.cond;
	     else
	       cond = bddfalse;
	     if ((hi != h.end() &&
		  (hi->second.in_stack || hi->second.processed)) &&
		 ((!conditions | cond) != bddtrue))
	       {
		 states->push_front(q_next);
	       }
	     else
	       delete q_next;
	   }
	 delete iter;
       }
     while (!states->empty());

     //delete s; // ??

     free_states();
  }

  void
  nesteddfsgen_search::print_stats(std::ostream& os) const
  {
    os << h.size() << " unique states visited" << std::endl;
    os << "size of stack : " << stack.size() << std::endl;
  }

}
