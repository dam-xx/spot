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

      assert(cycle.size() != 0);
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
  // The base interface for build a emptyness search algorithm
  emptyness_search::emptyness_search()
  {
  }

  emptyness_search::~emptyness_search()
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
  emptyness_search::print_stat(std::ostream& os) const
  {
    return os;
  }

  /////////////////////////////////////////////////////////////////////////////
  // minimal_search

  minimalce_search::minimalce_search(const tgba_tba_proxy *a)
    : a(a), min_ce(0)
  {
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
	if (*i == min_ce)
	  {
	    ++i;
	    continue;
	  }
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
    min_ce = new ce::counter_example(a);
    std::ostringstream os;
    const state* s = a->get_init_state();
    recurse_find(s, os);
    std::cout << "nb_found : " << nb_found << std::endl;

    if (min_ce->size() == 0)
      {
	delete min_ce;
	min_ce = 0;
      }

    tps_ = clock();
    return min_ce;
  }

  ce::counter_example*
  minimalce_search::check(ce::counter_example*)
  {
    min_ce = new ce::counter_example(a);

    /*
    ce::l_state_ce::iterator i;
    int depth = 0;
    for (i = min_ce->prefix.begin();
	 i != min_ce->prefix.end(); ++i, ++depth)
      {
	stack.push_front(i->first);
	//if (h_lenght.find(i->first) == h_lenght.end())
	h_lenght[i->first] = depth;
      }
    for (i = min_ce->cycle.begin();
	 i != min_ce->cycle.end(); ++i, ++depth)
      {
	stack.push_front(i->first);
	if (h_lenght.find(i->first) == h_lenght.end())
	  h_lenght[i->first] = depth;
      }
    */

    const state* s = a->get_init_state();
    std::ostringstream os;
    recurse_find(s, os);
    //if (recurse_find(s))
    //return min_ce;
    //else
    return min_ce;
  }

  void
  minimalce_search::recurse_find(const state* s,
				 std::ostringstream& os,
				 int mode)
  {
    std::cout << os.str() << "recurse find : "
	      << a->format_state(s) << std::endl;

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

    stack.push_front(state_pair(s, bddfalse));
    //stack.push_front(s);

    tgba_succ_iterator* iter = a->succ_iter(s);
    iter->first();
    while (!iter->done())
      {
	stack_type::iterator j = stack.begin();
	j->second = iter->current_condition();

	const state* succ = iter->current_state();

	std::cout << "stack.size() +1: " << (int)stack.size() + 1
		  << "min_ce->size() : " << min_ce->size()<< std::endl;
	if ((min_ce->size() == 0) ||
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
		     (min_ce->size() != 0))
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

    /*
    if (depth <= last_depth)
      std::cout << " : true => depth : "
		<< depth << ", last_depth"
		<< last_depth << std::endl;
    else
      std::cout << " : false => depth : "
		<< depth << ", last_depth : "
		<< last_depth << std::endl;
    */

    return depth <= last_depth; // May be '<='
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

    /*
    if (return_value)
      std::cout << " : true" << std::endl;
    else
      {
	depth = -1;
	std::cout << " : false" << std::endl;
      }
    */

    return depth;
  }

  void
  minimalce_search::save_counter(const state* s, std::ostringstream&)
  {
    //std::cout << os.str() << "save counter" << std::endl;

    nb_found++;
    if (min_ce->size())
      l_ce.push_front(min_ce);
    else
      delete min_ce;

    min_ce = new ce::counter_example(a);
    ce::state_ce ce;
    for (stack_type::iterator i = stack.begin();
	 i != stack.end(); ++i)
      {
	ce = ce::state_ce(i->first->clone(), i->second);
	//ce = ce::state_ce((*i)->clone(), bddfalse);
	min_ce->prefix.push_front(ce);
      }

    stack_type::iterator i = stack.begin();
    if (i == stack.end()) // empty counter example.
      return;

    //const state* s = *i;
    min_ce->build_cycle(s);
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

}
