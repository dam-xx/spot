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
	if ((*i).s)
	  delete (*i).s;
	if ((*i).lasttr)
	  delete (*i).lasttr;
      }
  }

  ce::counter_example*
  tarjan_on_fly::check()
  {
    std::cout << "tarjan_on_fly::check()" << std::endl;

    clock();

    top = dftop = -1;
    violation = false;

    const state* s = a->get_init_state();
    push(s);

    while (!violation && dftop >= 0)
      {
	//std::cout << "iter while" << std::endl;
	s = stack[dftop].s;
	std::cout << "s : " << a->format_state(s) << std::endl;
	tgba_succ_iterator* iter = stack[dftop].lasttr;
	if (iter == 0)
	  {
	    iter = a->succ_iter(s);
	    //std::cout << "iter->first" << std::endl;
	    iter->first();
	    stack[dftop].lasttr = iter;
	  }
	else
	  {
	    //std::cout << "iter->next" << std::endl;
	    iter->next();
	  }

	const state* succ = 0;
	if (!iter->done())
	  {
	    succ = iter->current_state();
	    if (h.find(succ) == h.end())
	      push(succ);
	    else
	      {
		int pos = in_stack(succ);
		delete succ;
		if (pos != -1) // succ is in stack
		  lowlinkupdate(dftop, pos);
	      }
	  }
	else
	  pop();
      }

    tps_ = clock();
    if (violation)
      return build_counter();

    //std::cout << "NO COUNTER EXAMPLE FOUND" << std::endl;

    return 0;
  }

  void
  tarjan_on_fly::push(const state* s)
  {
    std::cout << "tarjan_on_fly::push() : "
	      << a->format_state(s) << " : " << std::endl;

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

    /*
    std::cout << "    lowlink : " << ss.lowlink << std::endl;
    std::cout << "    pre     : " << ss.pre << std::endl;
    std::cout << "    acc     : " << ss.acc << std::endl;
    */

    if (top < (int)stack.size())
      {
	std::cout << "MAJ" << std::endl;

	/*
	const state* sdel = stack[top].s;
	tgba_succ_iterator* iter = stack[top].lasttr;
	*/

	stack[top] = ss;

	/*
	delete sdel;
	if (iter)
	  delete iter;
	*/

      }
    else
      {
	std::cout << "INS" << std::endl;
	stack.push_back(ss);
      }

    dftop = top;
  }

  void
  tarjan_on_fly::pop()
  {
    std::cout << "tarjan_on_fly::pop()" << std::endl;

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
    /*
    std::cout << "tarjan_on_fly::lowlinkupdate() : " << std::endl
	      << "     stack[t].lowlink : " << stack[t].lowlink
	      << "     stack[f].lowlink : " << stack[f].lowlink
	      << "     stack[f].acc     : " << stack[f].acc
	      << std::endl;
    */

    if (stack[t].lowlink <= stack[f].lowlink)
      {
	if (stack[t].lowlink <= stack[f].acc)
	  {
	    violation = true;
	    std::cout << "VIOLATION DETECTED" << std::endl;
	  }
	stack[f].lowlink = stack[t].lowlink;
      }
  }

  int
  tarjan_on_fly::in_stack(const state* s) const
  {
    std::cout << "tarjan_on_fly::in_stack() : "
	      << a->format_state(s) << std::endl;

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
    std::cout << "tarjan_on_fly::build_counter()" << std::endl;

    ce = new ce::counter_example(a);

    stack_type::iterator i;
    for (i = stack.begin(); i != stack.end(); ++i)
      {
	if (x && x->compare((*i).s) == 0)
	  break;

	//os << "  " << a->format_state(i->first) << std::endl;

	ce->prefix.push_back(ce::state_ce((*i).s->clone(),
					   //bddtrue));
					   (*i).lasttr->current_condition()));
      }

    for (; i != stack.end(); ++i)
      {
	//os << "  " << a->format_state(i->first) << std::endl;

	ce->cycle.push_back(ce::state_ce((*i).s->clone(),
					   //bddtrue));
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
       << "States explored : " << h.size() << std::endl
       << "Computed time : " << tps_ << " microseconds" << std::endl;
    return os;
  }

}
