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
#include "colordfs.hh"
#include "tgba/bddprint.hh"

namespace spot
{

  colordfs_search::colordfs_search(const tgba_tba_proxy* a)
    : a(a), x(0), counter_(0)
  {
  }

  colordfs_search::~colordfs_search()
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
  colordfs_search::push(const state* s, color c)
  {
    tgba_succ_iterator* i = a->succ_iter(s);
    i->first();

    /*
    hash_type::iterator hi = h.find(s);
    if (hi != h.end())
      if (hi->second.depth <= (int)stack.size())
	//return false; // FIXME
	return true;
    */

    color_state cs = { c, true , stack.size() }; // FIXME
    h[s] = cs;

    stack.push_front(state_iter_pair(s, i));

    // We build the counter example
    bdd b = bddfalse;
    if (!i->done()) // if the state is dead.
      b = i->current_condition();
    counter_->prefix.push_back(ce::state_ce(s->clone(), b));

    return true;
  }

  void
  colordfs_search::pop()
  {
    const state* s = stack.begin()->first;
    tgba_succ_iterator* i = stack.begin()->second;
    delete i;

    //std::cout << "pop : " << a->format_state(s) << std::endl;

    hash_type::iterator hi = h.find(s);
    assert(hi != h.end());
    hi->second.is_in_cp = false;
    stack.pop_front();
    //delete s;

    // We build the counter example
    delete counter_->prefix.back().first;
    counter_->prefix.pop_back();
  }

  bool
  colordfs_search::all_succ_black(const state* s)
  {
    bool return_value = true;
    hash_type::iterator hi;

    const state* s2;
    tgba_succ_iterator* i = a->succ_iter(s);
    int n = 0;
    for (i->first(); !i->done(); i->next(), n++)
      {
	//std::cout << "iter : " << n << std::endl;
	s2 = i->current_state();
	//std::cout << a->format_state(s2) << std::endl;
	hi = h.find(s2);
	if (hi != h.end())
	  return_value &= (hi->second.c == black);
	else
	  return_value = false;
	delete s2;
      }
    delete i;

    //std::cout << "End Loop" << std::endl;

    hi = h.find(s);
    assert(hi != h.end());
    if (return_value)
      hi->second.c = black;

    return return_value;
  }

  ce::counter_example*
  colordfs_search::check()
  {
    clock();
    counter_ = new ce::counter_example(a);
    const state* s = a->get_init_state();
    if (dfs_blue(s))
      counter_->build_cycle(x);
    else
      {
	delete counter_;
	counter_ = 0;
      }
    tps_ = clock();

    return counter_;
  }

  bool
  colordfs_search::dfs_blue(const state* s, bdd)
  {
    //std::cout << "dfs_blue : " << a->format_state(s) << std::endl;
    if (!push(s, blue))
      return false;

    hash_type::iterator hi;
    tgba_succ_iterator* i = a->succ_iter(s);
    int n = 0;
    for (i->first(); !i->done(); i->next(), n++)
      {
	const state* s2 = i->current_state();
	//std::cout << "s2 : " << a->format_state(s2) << std::endl;
	hi = h.find(s2);
	if (a->state_is_accepting(s2) &&
	    (hi != h.end() && hi->second.is_in_cp))
	  {
	    ce::state_ce ce;
	    ce = ce::state_ce(s2, i->current_condition());
	    x = const_cast<state*>(s2);
	    delete i;
	    return true;// a counter example is found !!
	  }
	else if (hi == h.end() || hi->second.c == white)
	  {
	    int res = dfs_blue(s2, i->current_acceptance_conditions());
	    if (res == 1)
	      {
		delete i;
		return true;
	      }
	  }
	else
	  delete s2; // FIXME
      }
    delete i;

    pop();

    if (!all_succ_black(s) &&
	a->state_is_accepting(s))
      {
	if (dfs_red(s))
	  return 1;
	dfs_black(s);
      }

    return false;
  }

  bool
  colordfs_search::dfs_red(const state* s)
  {
    //std::cout << "dfs_red : " << a->format_state(s) << std::endl;
    if (!push(s, red))
      return false;

    hash_type::iterator hi;
    tgba_succ_iterator* i = a->succ_iter(s);
    int n = 0;
    for (i->first(); !i->done(); i->next(), n++)
      {
	const state* s2 = i->current_state();
	hi = h.find(s2);
	if (hi != h.end() && hi->second.is_in_cp &&
	    (a->state_is_accepting(s2) ||
	     (hi->second.c == blue)))
	  {
	    //ce::state_ce ce;
	    //ce = ce::state_ce(s2->clone(), i->current_condition());
	    x = const_cast<state*>(s2);
	    delete i;
	    return true;// a counter example is found !!
	  }
	if (hi != h.end() && hi->second.c == blue)
	  {
	    delete s2; // FIXME
	    if (dfs_red(hi->first))
	      {
		delete i;
		return true;
	      }
	  }
	else
	  delete s2;
      }
    delete i;

    //std::cout << "dfs_red : pop" << std::endl;
    pop();

    return false;
  }

  void
  colordfs_search::dfs_black(const state* s)
  {
    //std::cout << "dfs_black" << a->format_state(s) << std::endl;
    hash_type::iterator hi = h.find(s);
    if (hi == h.end()) // impossible
      {
	color_state cs = { black, false, stack.size() };
	h[s] = cs;
      }
    else
      hi->second.c = black;

    tgba_succ_iterator* i = a->succ_iter(s);
    for (i->first(); !i->done(); i->next())
      {
	const state* s2 = i->current_state();
	hi = h.find(s2);
	if (hi == h.end())
	  {
	    color_state cs = { black, false, stack.size() };
	    h[s2] = cs;
	    dfs_black(s2);
	  }
	else
	  {
	    delete s2;
	    if (hi->second.c != black)
	      dfs_black(hi->first);
	  }
      }
    delete i;

  }

  std::ostream&
  colordfs_search::print_stat(std::ostream& os) const
  {
    int ce_size = 0;
    if (counter_)
      ce_size = counter_->size();
    os << "Size of Counter Example : " << ce_size << std::endl
       << "States explored : " << h.size() << std::endl
       << "Computed time : " << tps_ << " microseconds" << std::endl;
    return os;
  }

}
