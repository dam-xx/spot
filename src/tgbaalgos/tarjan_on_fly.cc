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

#include "tgbaalgos/tarjan_on_fly.hh"


namespace spot
{

  tarjan_on_fly::tarjan_on_fly(const tgba_tba_proxy *a)
    : a(a), x(0)
  {
  }

  tarjan_on_fly::~tarjan_on_fly()
  {
    for (stack_type::iterator i = stack.begin();
	 i != stack.end(); ++i)
      {
	//if ((*i).s)
	delete (*i).s;
	//if ((*i).lasttr)
	delete (*i).lasttr;
      }
  }

  ce::counter_example*
  tarjan_on_fly::check()
  {
    tgba_succ_iterator* iter = 0;
    const state* succ = 0;
    int pos = 0;
    top = dftop = -1;
    violation = false;

    const state* s = a->get_init_state();
    push(s);

    while (!violation && dftop >= 0)
      {
	s = stack[dftop].s;
	iter = stack[dftop].lasttr;
	if (iter == 0)
	  {
	    iter = a->succ_iter(s);
	    iter->first();
	    stack[dftop].lasttr = iter;
	  }
	else
	  {
	    iter->next();
	  }

	succ = 0;
	if (!iter->done())
	  {
	    succ = iter->current_state();
	    if (h.find(succ) == h.end())
	      push(succ);
	    else
	      {
		pos = in_stack(succ);
		delete succ;
		if (pos != -1) // succ is in stack
		  lowlinkupdate(dftop, pos);
	      }
	  }
	else
	  pop();
      }

    if (violation)
      return build_counter();

    //std::cout << "NO COUNTER EXAMPLE FOUND" << std::endl;

    return 0;
  }

  void
  tarjan_on_fly::push(const state* s)
  {
    h[s] = 1;
    top++;

    struct_state ss = { s, 0, top, dftop, 0, 0 };

    if (a->state_is_accepting(s))
      {
	ss.acc = top;
	x = s; // FIXME
      }
    else if (dftop >= 0)
      ss.acc = stack[dftop].acc;
    else
      ss.acc = -1;

    if (top < (int)stack.size())
      {
	const state* sdel = stack[top].s;
	tgba_succ_iterator* iter = stack[top].lasttr;
	delete sdel;
	if (iter)
	  delete iter;


	stack[top] = ss;
      }
    else
      {
	stack.push_back(ss);
      }

    dftop = top;
  }

  void
  tarjan_on_fly::pop()
  {
    int p = stack[dftop].pre;

    if (p >= 0)
      lowlinkupdate(p, dftop);

    if (stack[dftop].lowlink == dftop)
      top = dftop - 1;

    dftop = p;
  }

  void
  tarjan_on_fly::lowlinkupdate(int f, int t)
  {
    if (stack[t].lowlink <= stack[f].lowlink)
      {
	if (stack[t].lowlink <= stack[f].acc)
	  {
	    violation = true;
	  }
	stack[f].lowlink = stack[t].lowlink;
      }
  }

  int
  tarjan_on_fly::in_stack(const state* s) const
  {
    int n = 0;
    stack_type::const_iterator i;
    for (i = stack.begin(); i != stack.end(); ++i, ++n)
      if (s->compare((*i).s) == 0)
	break;

    if (i == stack.end())
      return -1;

    return n;
  }

  ce::counter_example*
  tarjan_on_fly::build_counter()
  {
    ce = new ce::counter_example(a);

    stack_type::iterator i;
    for (i = stack.begin(); i != stack.end(); ++i)
      {
	if (x && x->compare((*i).s) == 0)
	  break;

	ce->prefix.push_back(ce::state_ce((*i).s->clone(),
					  (*i).lasttr->current_condition()));
      }

    for (; i != stack.end(); ++i)
      {
	ce->cycle.push_back(ce::state_ce((*i).s->clone(),
					 (*i).lasttr->current_condition()));
      }

    return ce;
  }

  std::ostream&
  tarjan_on_fly::print_stat(std::ostream& os) const
  {
    int ce_size = 0;
    if (ce)
      ce_size = ce->size();
    os << "Size of Counter Example : " << ce_size << std::endl
       << "States explored : " << h.size() << std::endl;
    return os;
  }

}
