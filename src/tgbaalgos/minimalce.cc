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

#include "tgbaalgos/minimalce.hh"
#include "tgba/tgbaexplicit.hh"

namespace spot
{

  ///////////////////////////////////////////////////////////////////////////
  // Class counter example.

  namespace ce
  {

    counter_example::counter_example(const tgba* a)
      : automata_(a)
    {
    }

    counter_example::~counter_example()
    {
      for (l_state_ce::const_iterator i = prefix.begin();
	   i != prefix.end(); ++i)
	{
	  delete i->first;
	}
      for (l_state_ce::const_iterator i = cycle.begin();
	   i != cycle.end(); ++i)
	{
	  delete i->first;
	}
    }

    void
    counter_example::build_cycle(const state* x)
    {
      if (!x)
	return;
      bool in_cycle = false;
      for (l_state_ce::iterator i = prefix.begin();
	   i != prefix.end();)
	{
	  if (i->first->compare(x) == 0)
	    in_cycle = true;
	  if (in_cycle)
	    {
	      cycle.push_back(*i);
	      i = prefix.erase(i);
	    }
	  else
	    ++i;
	}
    }

    int
    counter_example::size()
    {
      return prefix.size() + cycle.size();
    }

    std::ostream&
    counter_example::print(std::ostream& os) const
    {
      os << "Prefix:" << std::endl;
      const bdd_dict* d = automata_->get_dict();
      for (l_state_ce::const_iterator i = prefix.begin();
	   i != prefix.end(); ++i)
	{
	  os << "  " << automata_->format_state(i->first) << std::endl;
	  os << "    | " << bdd_format_set(d, i->second) << std::endl;
	}
      os <<"Cycle:" <<std::endl;
      for (l_state_ce::const_iterator i = cycle.begin();
	   i != cycle.end(); ++i)
	{
	  os << "  " << automata_->format_state(i->first) << std::endl;
	  os << "    | " << bdd_format_set(d, i->second) << std::endl;
	}
      return os;
    }

    bdd_dict*
    counter_example::get_dict() const
    {
      return automata_->get_dict();
    }

    void
    counter_example::project_ce(const tgba* aut, std::ostream& os)
    {
      os << "prefix :" << std::endl;
      for (ce::l_state_ce::const_iterator i = prefix.begin();
	   i != prefix.end(); ++i)
	{
	  const state* s = aut->project_state(i->first, aut);
	  assert(s);
	  os << aut->format_state(s) << std::endl;
	  delete s;
	}

      os << "cycle :" << std::endl;
      for (ce::l_state_ce::const_iterator i = cycle.begin();
	   i != cycle.end(); ++i)
	{
	  const state* s = aut->project_state(i->first, aut);
	  assert(s);
	  os << aut->format_state(s) << std::endl;
	  delete s;
	}

    }

    tgba*
    counter_example::ce2tgba()
    {

      tgba_explicit* aut = new tgba_explicit(automata_->get_dict());

      std::string strs, strd;
      tgba_explicit::transition* t;
      ce::l_state_ce::const_iterator is = prefix.begin();
      ce::l_state_ce::const_iterator id = is;
      //ce::l_state_ce::const_iterator is_c;
      //ce::l_state_ce::const_iterator id_c;
      if (is != prefix.end())
	{
	  strs = automata_->format_state(is->first);
	  aut->set_init_state(strs);
	  ++id;

	  for (; id != prefix.end(); ++is, ++id)
	    {
	      strs = automata_->format_state(is->first);
	      strd = automata_->format_state(id->first);
	      t = aut->create_transition(strs, strd);
	      aut->add_conditions(t, is->second);
	    }

	  id = cycle.begin();
	}
      else
	{
	  id = cycle.begin();
	  is = id;
	  ++id;
	}

      for (; id != cycle.end();)
	{
	  strs = automata_->format_state(is->first);
	  strd = automata_->format_state(id->first);
	  t = aut->create_transition(strs, strd);
	  aut->add_conditions(t, is->second);
	  is = id;
	  ++id;
	}

      assert(!cycle.empty());
      is = cycle.end();
      is--;
      id = cycle.begin();
      strs = automata_->format_state(is->first);
      strd = automata_->format_state(id->first);
      t = aut->create_transition(strs, strd);
      aut->add_conditions(t, is->second);

      aut->merge_transitions();

      return aut;
    }

  }

  /////////////////////////////////////////////////////////////////////////////
  // The base interface for build a emptiness search algorithm
  emptiness_search::emptiness_search()
  {
  }

  emptiness_search::~emptiness_search()
  {
  }

  /*
  std::ostream&
  nesteddfs_search::print_stat(std::ostream& os) const
  {
    int ce_size = 0;
    if (counter_)
      ce_size = counter_->size();
    os << "Size of Counter Example : " << ce_size << std::endl
    os << "States explored : " << size_ << std::endl
       << "Computed time : " << tps_ << " microseconds" << std::endl;
    return os;
  }
  */

  std::ostream&
  emptiness_search::print_stat(std::ostream& os) const
  {
    return os;
  }

  /////////////////////////////////////////////////////////////////////////////
  // minimal_search

  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////

  minimalce_search::minimalce_search(const tgba_tba_proxy* a,
				     int opt)
    : a(a), x(0),
      x_bis(0),
      accepted_path_(false)
  {
    counter_ = 0;
    nested_ = my_nested_ = false;
    if (opt == nested)
      nested_ = true;
    if (opt == my_nested)
      my_nested_ = true;
    Maxsize = 0;
  }

  minimalce_search::~minimalce_search()
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
    for (std::list<ce::counter_example*>::iterator i = l_ce.begin();
	 i != l_ce.end(); ++i)
      {
	delete *i;
      }
  }


  bool
  minimalce_search::push(const state* s, bool m)
  {
    if ((Maxsize != 0) && // for minimize
	(stack.size() + 1 > Maxsize))
      return false;

    tgba_succ_iterator* i = a->succ_iter(s);
    i->first();

    hash_type::iterator hi = h.find(s);
    if (hi == h.end())
      {
	magic d = { !m, m, true, stack.size() + 1};
	//magic d = { !m, m, true };
	h[s] = d;
      }
    else
      {
	hi->second.seen_without |= !m;
	hi->second.seen_with |= m;
	hi->second.seen_path = true; // for nested search

	if ((stack.size() + 1) < hi->second.depth) // for minimize
	  hi->second.depth = stack.size() + 1;

	if (hi->first != s)
	  delete s;
	s = hi->first;
      }

    magic_state ms = { s, m };
    stack.push_front(state_iter_pair(ms, i));

    return true;
  }

  bool
  minimalce_search::has(const state* s, bool m) const
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
  minimalce_search::exist_path(const state* s) const
  {
    hash_type::const_iterator hi = h.find(s);
    if (hi == h.end())
      return false;
    if (hi->second.seen_with)
      return false;
    return hi->second.seen_path && hi->second.seen_without;
  }

  int
  minimalce_search::depth_path(const state* s) const
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
  minimalce_search::check()
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
	minimalce_search::state_iter_pair& p = stack.front();
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
		Maxsize = stack.size();
		counter_->build_cycle(x);
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
  minimalce_search::print_result(std::ostream& os, const tgba* restrict) const
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
  minimalce_search::print_stat(std::ostream& os) const
  {
    int ce_size = 0;
    if (counter_)
      ce_size = counter_->size();
    os << "Size of Counter Example : " << ce_size << std::endl
       << "States explored : " << h.size() << std::endl;
    return os;
  }

  void
  minimalce_search::build_counter()
  {
    if (counter_)
      l_ce.push_front(counter_);
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

  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  /*
  minimalce_search::minimalce_search(const tgba_tba_proxy *a,
				     bool mode)
    : a(a), min_ce(0),
      x(0),
      x_bis(0),
      accepted_path_(false)
  {
    Maxsize = 0;
    nested_ = my_nested_ = false;
    mode_ = mode;
  }

  minimalce_search::~minimalce_search()
  {
    hash_type::const_iterator s = h_lenght.begin();
    while (s != h_lenght.end())
      {
	// Advance the iterator before deleting the "key" pointer.
	const state* ptr = s->first;
	++s;
	delete ptr;
      }

    for (std::list<ce::counter_example*>::iterator i = l_ce.begin();
	 i != l_ce.end();)
      {
	//std::cout << "delete a counter" << std::endl;

	//if (*i == min_ce)
	//{
	//++i;
	//continue;
	//}

	ce::counter_example* ce = *i;
	++i;
	delete ce;
      }

    //std::cout << "END ~minimalce_search()" << std::endl;
  }

  ce::counter_example*
  minimalce_search::check()
  {
    clock();
    nb_found = 0;

    if ((!min_ce && mode_) ||
	!mode_)
      min_ce = new ce::counter_example(a);

    if (mode_)
      {
	min_ce = find();
	tps_ = clock();
	return min_ce;
      }

    std::ostringstream os;
    const state* s = a->get_init_state();
    recurse_find(s, os);
    //std::cout << "nb_found : " << nb_found << std::endl;

    if (min_ce->empty())
      {
	delete min_ce;
	min_ce = 0;
      }

    tps_ = clock();
    return min_ce;
  }

  ce::counter_example*
  minimalce_search::check(ce::counter_example* ce)
  {
    min_ce = ce;

    std::ostringstream os;
    const state* s = a->get_init_state();
    recurse_find(s, os);
    //std::cout << "nb_found : " << nb_found << std::endl;
    tps_ = clock();
    return min_ce;
  }

  ce::counter_example*
  minimalce_search::find()
  {
    /// FIXME
    std::ostringstream os;

    int mode = normal;
    int depth_mode = 0;
    int depth_mode_memory = -1;
    const state* s = 0;
    hash_type::iterator i;
    tgba_succ_iterator* iter = 0;

    if (h_lenght.empty())
      {
	// it's a new search
	//std::cout << "it's a new search" << std::endl;
	s = a->get_init_state();
	i = h_lenght.find(s);
	if (i != h_lenght.end())
	  {
	    delete s;
	    s = i->first;
	    if (((int)stack.size() + 1) < i->second)
	      i->second = stack.size() + 1;
	  }
	else
	  h_lenght[s] = stack.size();
	iter = a->succ_iter(s);
	iter->first();
	stack.push_front(state_pair(s, iter));
	depth_mode++;
      }
    else
      s = stack.front().first;

    while (!stack.empty())
      {
      recurse:
	//std::cout << "recurse: " << a->format_state(s) << std::endl;

	// if (iter)
	// delete iter;

	iter = stack.front().second;
	while (!iter->done())
	  {
	    //std::cout << "iter" << std::endl;

	    //stack_type::iterator j = stack.begin();
	    //j->second = iter->current_condition();

	    const state* succ = iter->current_state();

	    if (min_ce->empty() ||
		((int)stack.size() + 1 <= min_ce->size()))
	      {
		int depth = in_stack(succ, os);
		if (depth != -1)
		  {
		    if (closes_accepting(succ, depth, os))
		      {
			// New counter example is found !!
			//std::cout << "CE found !!" << std::endl;
			save_counter(succ, os);

			i = h_lenght.find(succ);
			if (i == h_lenght.end())
			  h_lenght[succ] = stack.size() + 1;
			else
			  {
			    delete succ;
			    if (((int)stack.size() + 1) < i->second)
			      i->second = stack.size() + 1;
			  }

			iter->next();
			return min_ce;
		      }
		    else
		      delete succ;
		  }

		else if ((mode == careful) ||
			 a->state_is_accepting(succ))
		  {
		    s = succ;
		    iter->next();

		    if (mode != careful)
		      depth_mode_memory = depth_mode;
		    mode = careful;
		    hash_type::iterator i = h_lenght.find(s);
		    if (i != h_lenght.end())
		      {
			delete s;
			s = i->first;
			if (((int)stack.size() + 1) < i->second)
			  i->second = stack.size() + 1;
		      }
		    else
		      h_lenght[s] = stack.size();

		    iter = a->succ_iter(s);
		    iter->first();
		    stack.push_front(state_pair(s, iter));
		    depth_mode++;
		    goto recurse;
		  }

		else if (h_lenght.find(succ) == h_lenght.end())
		  {
		    s = succ;
		    iter->next();

		    hash_type::iterator i = h_lenght.find(s);
		    if (i != h_lenght.end())
		      {
			delete s;
			s = i->first;
			if (((int)stack.size() + 1) < i->second)
			  i->second = stack.size() + 1;
		      }
		    else
		      h_lenght[s] = stack.size();

		    iter = a->succ_iter(s);
		    iter->first();
		    stack.push_front(state_pair(s, iter));
		    depth_mode++;
		    goto recurse;
		  }

		else if ((h_lenght[succ] > (int)stack.size() + 1) &&
			 !min_ce->empty())
		  {
		    s = succ;
		    iter->next();

		    if (mode != careful)
		      depth_mode_memory = depth_mode;
		    mode = careful;
		    hash_type::iterator i = h_lenght.find(s);
		    if (i != h_lenght.end())
		      {
			delete s;
			s = i->first;
			if (((int)stack.size() + 1) < i->second)
			  i->second = stack.size() + 1;
		      }
		    else
		      h_lenght[s] = stack.size();

		    iter = a->succ_iter(s);
		    iter->first();
		    stack.push_front(state_pair(s, iter));
		    depth_mode++;
		    goto recurse;
		  }
		else
		  delete succ;
	      }
	    else
	      delete succ;

	    iter->next();
	  }

	delete iter;

	depth_mode--;
	if (depth_mode_memory == depth_mode)
	  {
	    depth_mode_memory = -1;
	    mode = normal;
	  }

	stack.pop_front();
	s = stack.front().first;
      }

    return 0;
  }

  void
  minimalce_search::recurse_find(const state* s,
				 std::ostringstream& os,
				 int mode)
  {

    // std::cout << os.str() << "recurse find : "
    // << a->format_state(s) << std::endl;

    hash_type::iterator i = h_lenght.find(s);
    if (i != h_lenght.end())
      {
	delete s;
	s = i->first;
	if (((int)stack.size() + 1) < i->second)
	  i->second = stack.size() + 1;
      }
    else
      h_lenght[s] = stack.size() + 1;

    //stack.push_front(state_pair(s, bddfalse));

    tgba_succ_iterator* iter = a->succ_iter(s);
    stack.push_front(state_pair(s, iter));
    iter->first();
    while (!iter->done())
      {
	//stack_type::iterator j = stack.begin();
	//j->second = iter->current_condition();

	const state* succ = iter->current_state();

	if (min_ce->empty() ||
	    ((int)stack.size() + 1 <= min_ce->size()))
	  {
	    int depth = in_stack(succ, os);
	    if (depth != -1)
	      {
		if (closes_accepting(succ, depth, os))
		  {
		    // New counter example is found !!
		    save_counter(succ, os);

		    i = h_lenght.find(succ);
		    if (i == h_lenght.end())
		      h_lenght[succ] = stack.size() + 1;
		    else
		      {
			delete succ;
			if (((int)stack.size() + 1) < i->second)
			  i->second = stack.size() + 1;
		      }

		  }
		else
		  delete succ;
	      }
	    else if ((mode == careful) ||
		     a->state_is_accepting(succ))
	      {
		//std::cout << "recurse 1 : " << stack.size() << " ";
		mode = careful;
		os << "  ";
		recurse_find(succ, os, mode);
	      }
	    else if (h_lenght.find(succ) == h_lenght.end())
	      {
		//std::cout << "recurse 2 : " << stack.size() << " ";
		os << "  ";
		recurse_find(succ, os, mode);
	      }
	    else if ((h_lenght[succ] > (int)stack.size() + 1) &&
		     !min_ce->empty())
	      {
		//std::cout << "recurse 3 : " << stack.size() << " ";
		mode = careful;
		os << "  ";
		recurse_find(succ, os, mode);
	      }
	    else
	      delete succ;
	  }
	else //if (h_lenght.find(succ) == h_lenght.end())
	  delete succ;

	iter->next();
      }

    delete iter;

    //std::cout << os.str() << "stack.pop_front()" << std::endl;
    stack.pop_front();

  }

  bool
  minimalce_search::closes_accepting(const state*,
				     int depth,
				     std::ostringstream&) const
  {
    //std::cout << os.str() << "close accepting : " << a->format_state(s);

    int last_depth = -1;
    int depth_cp = stack.size();

    for (stack_type::const_iterator i = stack.begin();
	 i != stack.end(); ++i, --depth_cp)
      if (a->state_is_accepting(i->first))
	//if (a->state_is_accepting(*i))
	{
	  last_depth = depth_cp - 1;
	  //last_depth = h_lenght[*i];
	  break;
	}

    return depth <= last_depth;
  }

  int
  minimalce_search::in_stack(const state* s, std::ostringstream&) const
  {
    //std::cout << os.str() << "in stack : " << a->format_state(s);

    int depth = stack.size();

    bool return_value = false;
    for (stack_type::const_iterator i = stack.begin();
	 i != stack.end() && !return_value; ++i, --depth)
      {
	if (s->compare(i->first) == 0)
	  //if (s->compare(*i) == 0)
	  return_value = true;
      }

    if (!return_value)
      depth = -1;

    return depth;
  }

  void
  minimalce_search::save_counter(const state* s, std::ostringstream&)
  {
    //std::cout << os.str() << "save counter" << std::endl;

    nb_found++;
    if (min_ce->empty())
      delete min_ce;

    min_ce = new ce::counter_example(a);
    ce::state_ce ce;
    for (stack_type::iterator i = stack.begin();
	 i != stack.end(); ++i)
      {
	//ce = ce::state_ce(i->first->clone(), i->second->current_condition());
	ce = ce::state_ce(i->first->clone(), bddfalse);
	min_ce->prefix.push_front(ce);
      }

    stack_type::iterator i = stack.begin();
    if (i == stack.end()) // empty counter example.
      return;

    //const state* s = *i;
    min_ce->build_cycle(s);
    l_ce.push_front(min_ce);
  }

  std::ostream&
  minimalce_search::print_stat(std::ostream& os) const
  {
    int ce_size = 0;
    if (min_ce)
      ce_size = min_ce->size();
    os << "Size of Counter Example : " << ce_size << std::endl
       << "States explored : " << h_lenght.size() << std::endl
       << "Computed time : " << tps_ << " microseconds" << std::endl;
    return os;
  }

  ce::counter_example*
  minimalce_search::get_minimal_cyle() const
  {
    ce::counter_example* min_cycle = min_ce;
    for (std::list<ce::counter_example*>::const_iterator i = l_ce.begin();
	 i != l_ce.end();)
      if ((*i)->cycle.size() < min_cycle->cycle.size())
	min_cycle = *i;
    return min_cycle;
  }

  ce::counter_example*
  minimalce_search::get_minimal_prefix() const
  {
    ce::counter_example* min_prefix = min_ce;
    for (std::list<ce::counter_example*>::const_iterator i = l_ce.begin();
	 i != l_ce.end();)
      if ((*i)->prefix.size() < min_prefix->prefix.size())
	min_prefix = *i;
    return min_prefix;
  }
  */

}
