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

#include <iterator>
#include <cassert>
#include "magic.hh"
#include "tgba/bddprint.hh"

namespace spot
{
  magic_search::result::result(magic_search& ms)
    : ms_(ms)
  {
  }

  tgba_run*
  magic_search::result::accepting_run()
  {
    tgba_run* run = new tgba_run;

    stack_type::const_reverse_iterator i, e = ms_.stack.rend();
    tstack_type::const_reverse_iterator ti;
    tgba_run::steps* l = &run->prefix;

    for (i = ms_.stack.rbegin(), ti = ms_.tstack.rbegin(); i != e; ++i, ++ti)
      {
	if (i->first.s->compare(ms_.x) == 0)
	  l = &run->cycle;

	// FIXME: We need to keep track of the acceptance condition.
	tgba_run::step s = { i->first.s->clone(), *ti, bddfalse };
	l->push_back(s);
      }

    return run;
  }


  magic_search::magic_search(const tgba_tba_proxy* a)
    : a(a), x(0)
  {
  }

  magic_search::~magic_search()
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

  void
  magic_search::push(const state* s, bool m)
  {
    tgba_succ_iterator* i = a->succ_iter(s);
    i->first();

    hash_type::iterator hi = h.find(s);
    if (hi == h.end())
      {
	magic d = { !m, m };
	h[s] = d;
      }
    else
      {
	hi->second.seen_without |= !m;
	hi->second.seen_with |= m;
	if (hi->first != s)
	  delete s;
	s = hi->first;
      }

    magic_state ms = { s, m };
    stack.push_front(state_iter_pair(ms, i));
  }

  bool
  magic_search::has(const state* s, bool m) const
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

  emptiness_check_result*
  magic_search::check()
  {
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
	magic_search::state_iter_pair& p = stack.front();
	tgba_succ_iterator* i = p.second;
	const bool magic = p.first.m;

	while (!i->done())
	  {
	    const state* s_prime = i->current_state();
	    bdd c = i->current_condition();
	    i->next();
	    if (magic && 0 == s_prime->compare(x))
	      {
		delete s_prime;
		tstack.push_front(c);
		assert(stack.size() == tstack.size());
		return new result(*this);
	      }
	    if (!has(s_prime, magic))
	      {
		push(s_prime, magic);
		tstack.push_front(c);
		goto recurse;
	      }
	    delete s_prime;
	  }

	const state* s = p.first.s;
	delete i;
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

    assert(tstack.empty());
    return 0;
  }

}
