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

#include <iterator>
#include <cassert>
#include "nesteddfs.hh"
#include "tgba/bddprint.hh"

namespace spot
{
  nesteddfs_search::nesteddfs_search(const tgba_tba_proxy* a,
				     int opt)
    : a(a), x(0),
      x_bis(0),
      accepted_path_(false)
  {
    nested_ = my_nested_ = false;
    if (opt == nested)
      nested_ = true;
    if (opt == my_nested)
      my_nested_ = true;
    Maxsize = 0;
  }

  nesteddfs_search::~nesteddfs_search()
  {
    hash_type::const_iterator s = h.begin();
    while (s != h.end())
      {
	// Advance the iterator before deleting the "key" pointer.
	const state* ptr = s->first;
	++s;
	delete ptr;
      }
    if (x)
      delete x;
    // Release all iterators on the stack.
    while (!stack.empty())
      {
	delete stack.front().second;
	stack.pop_front();
      }
  }

  bool
  nesteddfs_search::push(const state* s, bool m)
  {
    /*
      if ((Maxsize != 0) && // for minimize
      (stack.size() + 1 > Maxsize))
      return false;
    */

    tgba_succ_iterator* i = a->succ_iter(s);
    i->first();

    hash_type::iterator hi = h.find(s);
    if (hi == h.end())
      {
	//magic d = { !m, m, true, stack.size() + 1};
	magic d = { !m, m, true };
	h[s] = d;
      }
    else
      {
	hi->second.seen_without |= !m;
	hi->second.seen_with |= m;
	hi->second.seen_path = true; // for nested search
	/*
	  if ((stack.size() + 1) < hi->second.depth) // for minimize
	  hi->second.depth = stack.size() + 1;
	*/
	if (hi->first != s)
	  delete s;
	s = hi->first;
      }

    magic_state ms = { s, m };
    stack.push_front(state_iter_pair(ms, i));

    return true;
  }

  bool
  nesteddfs_search::has(const state* s, bool m) const
  {
    hash_type::const_iterator i = h.find(s);
    if (i == h.end())
      return false;
    if (!m && i->second.seen_without)
      return true;
    if (m && i->second.seen_with)
      return true;
    return false;
  }

  bool
  nesteddfs_search::exist_path(const state* s) const
  {
    hash_type::const_iterator hi = h.find(s);
    if (hi == h.end())
      return false;
    if (hi->second.seen_with)
      return false;
    return hi->second.seen_path && hi->second.seen_without;
  }

  int
  nesteddfs_search::depth_path(const state* s) const
  {
    int depth = 0;
    stack_type::const_reverse_iterator i;
    for (i = stack.rbegin(); i != stack.rend(); ++i, ++depth)
      if (s->compare(i->first.s) == 0)
	break;

    if (i != stack.rend())
      return depth;
    else
      return stack.size() + 1;
  }

  ce::counter_example*
  nesteddfs_search::check()
  {
    if (my_nested_)
      {
	accepted_path_ = false;
	accepted_depth_ = 0;
      }

    if (stack.empty())
      // It's a new search.
      push(a->get_init_state(), false);
    else
      // Remove the transition to the cycle root.
      tstack.pop_front();

    assert(stack.size() == 1 + tstack.size());

    while (!stack.empty())
      {
      recurse:
	//std::cout << "recurse : "<< stack.size() << std::endl;
	nesteddfs_search::state_iter_pair& p = stack.front();
	tgba_succ_iterator* i = p.second;
	const bool magic = p.first.m;

	while (!i->done())
	  {
	    const state* s_prime = i->current_state();
	    //std::cout << a->format_state(s_prime) << std::endl;
	    bdd c = i->current_condition();
	    i->next();

	    if ((magic && 0 == s_prime->compare(x)) ||
		(magic && (nested_ || my_nested_) && exist_path(s_prime)) ||
		(!magic && my_nested_ && accepted_path_ &&
		 exist_path(s_prime) && depth_path(s_prime) <= accepted_path_))
	      {
		if (nested_ || my_nested)
		  {
		    if (x)
		      delete x;
		    x = s_prime->clone();
		  }
		delete s_prime;
		tstack.push_front(c);
		assert(stack.size() == tstack.size());

		build_counter();
		//Maxsize = stack.size();
		//counter_->build_cycle(x);
		return counter_;
	      }
	    if (!has(s_prime, magic))
	      {
		if (my_nested_ && a->state_is_accepting(s_prime))
		  {
		    accepted_path_ = true;
		    accepted_depth_ = stack.size();
		  }
		if (push(s_prime, magic))
		  {
		    tstack.push_front(c);
		    goto recurse;
		  }
		// for minimize
	      }
	    delete s_prime;
	  }

	const state* s = p.first.s;
	delete i;
	if (nested_ || my_nested_)
	  {
	    hash_type::iterator hi = h.find(((stack.front()).first).s);
	    assert (hi != h.end());
	    hi->second.seen_path = false;
	  }
	stack.pop_front();

	if (!magic && a->state_is_accepting(s))
	  {
	    if (!has(s, true))
	      {
		if (x)
		  delete x;
		x = s->clone();
		push(s, true);
		continue;
	      }
	  }
	if (!stack.empty())
	  tstack.pop_front();
      }

    std::cout << "END CHECK" << std::endl;

    assert(tstack.empty());

    return 0;
  }

  std::ostream&
  nesteddfs_search::print_result(std::ostream& os, const tgba* restrict) const
  {
    stack_type::const_reverse_iterator i;
    tstack_type::const_reverse_iterator ti;
    os << "Prefix:" << std::endl;
    const bdd_dict* d = a->get_dict();
    for (i = stack.rbegin(), ti = tstack.rbegin();
	 i != stack.rend(); ++i, ++ti)
      {
	if (i->first.s->compare(x) == 0)
	  os <<"Cycle:" <<std::endl;

	const state* s = i->first.s;
	if (restrict)
	  {
	    s = a->project_state(s, restrict);
	    assert(s);
	    os << "  " << restrict->format_state(s) << std::endl;
	    delete s;
	  }
	else
	  {
	    os << "  " << a->format_state(s) << std::endl;
	  }
	os << "    | " << bdd_format_set(d, *ti) << std::endl;
      }

    if (restrict)
      {
	const state* s = a->project_state(x, restrict);
	assert(s);
	os << "  " << restrict->format_state(s) << std::endl;
	delete s;

      }
    else
      {
	os << "  " << a->format_state(x) << std::endl;
      }
    return os;
  }

  std::ostream&
  nesteddfs_search::print_stat(std::ostream& os) const
  {
    int ce_size = 0;
    if (counter_)
      ce_size = counter_->size();
    os << "Size of Counter Example : " << ce_size << std::endl
       << "States explored : " << h.size() << std::endl;
    return os;
  }

  void
  nesteddfs_search::build_counter()
  {
    assert(stack.size() == tstack.size());
    counter_ = new ce::counter_example(a);
    stack_type::reverse_iterator i;
    tstack_type::reverse_iterator ti;
    for (i = stack.rbegin(), ti = tstack.rbegin();
	 i != stack.rend(); ++i, ++ti)
      {
	if (i->first.s->compare(x) == 0)
	  break;
	ce::state_ce ce;
	ce = ce::state_ce(i->first.s->clone(), *ti);
	counter_->prefix.push_back(ce);
      }
    for (; i != stack.rend(); ++i, ++ti)
      {
	ce::state_ce ce;
	ce = ce::state_ce(i->first.s->clone(), *ti);
	counter_->cycle.push_back(ce);
      }
    //counter_->build_cycle(x);
  }

}
