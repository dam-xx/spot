// Copyright (C) 2006 Laboratoire d'Informatique de
// Paris 6 (LIP6), département Systèmes Répartis Coopératifs (SRC),
// Université Pierre et Marie Curie.
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

#include "sync.hh"
#include <iostream>
#include <iterator>
#include "misc/hash.hh"
#include "misc/hashfunc.hh"
#include "misc/modgray.hh"


#define PTR_TO_INT(p) \
  (reinterpret_cast<const char*>(p) - static_cast<const char*>(0))

namespace spot
{
  struct sync_state_
  {
    sync_state_(sync::vnodes& v)
      : nodes(v)
    {
      size_t h = 0;
      for (sync::vnodes::const_iterator i = v.begin(); i != v.end(); ++i)
	h ^= knuth32_hash(PTR_TO_INT(*i));
      hash_key = h;
    }

    sync::vnodes nodes;
    size_t hash_key;

    int
    compare(const sync_state_* o) const
    {
      if (hash_key < o->hash_key)
	return -1;
      if (hash_key > o->hash_key)
	return 1;
      sync::vnodes::const_iterator i = nodes.begin();
      sync::vnodes::const_iterator j = o->nodes.begin();
      for (; i != nodes.end(); ++i, ++j)
	{
	  if (*i < *j)
	    return -1;
	  if (*i > *j)
	    return 1;
	}
      return 0;
    }
  };

  struct sync_state_hash
  {
    size_t
    operator()(const sync_state_& s) const
    {
      return s.hash_key;
    }
  };

  struct sync_state_eq
  {
    size_t
    operator()(const sync_state_& s1, const sync_state_& s2) const
    {
      return s1.nodes == s2.nodes;
    }
  };

  class sync_state_heap
  {
    typedef Sgi::hash_set<sync_state_, sync_state_hash, sync_state_eq> heap_t;
    heap_t heap;
  public:
    const sync_state_*
    insert(sync::vnodes nodes)
    {
      assert(nodes.size() == 2);
      std::pair<heap_t::iterator, bool> p =
	heap.insert(sync_state_(nodes));
      if (p.second)
	{
	  std::cerr << "heap " << this << " inserts " << &*p.first << " =";
	  for (sync::vnodes::const_iterator i = nodes.begin();
	       i != nodes.end(); ++i)
	    std::cerr << " " << *i;
	  std::cerr << std::endl;
	}
      else
	{
	  std::cerr << "heap " << this << " caches " << &*p.first << std::endl;
	}
      return &*p.first;
    }
  };

  struct sync_state : public state
  {
    sync_state(const sync_state_* s) : s(s) {}
    const sync_state_* s;

    int
    compare(const spot::state* o_) const
    {
      const sync_state* o = dynamic_cast<const sync_state*>(o_);
      assert(o);
      return s->compare(o->s);
    }

    size_t
    hash() const
    {
      return s->hash_key;
    }

    sync_state*
    clone() const
    {
      return new sync_state(*this);
    }

  };



  sync::sync(saut_list& sautlist, bdd_dict* dict)
    : auts(sautlist.begin(), sautlist.end()),
      autsk(auts.size()),
      autssize(auts.size()),
      heap(new sync_state_heap),
      dict(dict)
  {
    assert(!sautlist.empty());

    std::cerr << "sync " << this << " created for (";
    for (unsigned n = 0;;)
      {
	autsk[n] = knuth32_hash(PTR_TO_INT(auts[n] + n));
	std::cerr << auts[n];
	if (++n == autssize)
	  break;
	std::cerr << ", ";
      }
    std::cerr << ") with heap " << heap << std::endl;
  }

  bool
  sync::known_action(unsigned aut_num, const saut::action_name& act) const
  {
    assert(aut_num < autssize);
    return auts[aut_num]->known_action(act);
  }

  bool
  sync::declare_rule(action_list& l)
  {
    assert(l.size() == autssize);

    std::cerr << "sync " << this << " declares rule (";
    action_list::const_iterator i = l.begin();
    unsigned k = autssize;
    unsigned j = 0;
    bool not_epsilon = false;
    action_vect v(autssize);
    size_t hk = 0;
    for (;;)
      {
	assert(i != l.end());
	const saut::action* a;
	if (*i)
	  {
	    a = auts[j]->known_action(**i);
	    assert(a);
	    not_epsilon = true;
	    v[j] = a;
	    hk ^= knuth32_hash(autsk[j] ^ PTR_TO_INT(a));
	  }
	else
	  {
	    a = 0;
	  }
	std::cerr << a;
	if (++j >= k)
	  break;
	++i;
	std::cerr << ", ";
      }
    std::cerr << ")";
    if (not_epsilon)
      {
	actions.insert(action_map::value_type(hk, v));
	std::cerr << " as #" << hk;
      }
    std::cerr << std::endl;
    return !not_epsilon;
  }

  state*
  sync::get_init_state() const
  {
    vnodes v(autssize);
    for (unsigned n = 0; n < autssize; ++n)
      v[n] = auts[n]->get_initial();
    return new sync_state(heap->insert(v));
  }

  bdd_dict*
  sync::get_dict() const
  {
    return dict;
  }

  std::string
  sync::format_state(const vnodes& nodes) const
  {
    std::string r;
    for (unsigned n = 0;;)
      {
	r += *nodes[n]->name;
	if (++n == autssize)
	  break;
	r += '*';
      }
    return r;
  }

  std::string
  sync::format_state(const state* s) const
  {
    const sync_state* s_ = dynamic_cast<const sync_state*>(s);
    return format_state(s_->s->nodes);
  }

  bdd
  sync::all_acceptance_conditions() const
  {
    return bddfalse;
  }

  bdd
  sync::neg_acceptance_conditions() const
  {
    return bddfalse;
  }

  bdd
  sync::compute_support_conditions(const state*) const
  {
    return bddtrue;
  }

  bdd
  sync::compute_support_variables(const state*) const
  {
    return bddfalse;
  }

  struct sync_transitions :
    public tgba_succ_iterator,
    public loopless_modular_mixed_radix_gray_code
  {
    typedef std::vector<saut::transitions_list::const_iterator> array;
    unsigned size;
    array pos;
    size_t hk;
    const sync::vnodes& nodes;
    const sync& syn;

    sync_transitions(const sync::vnodes& nodes, const sync& syn)
      : loopless_modular_mixed_radix_gray_code(nodes.size()),
	size(nodes.size()),
	pos(nodes.size()),
	nodes(nodes),
	syn(syn)
    {
      hk = 0;
      for (unsigned n = 0; n < size; ++n)
	{
	  pos[n] = nodes[n]->out.begin();
	  if (pos[n] != nodes[n]->out.end())
	    hk ^= knuth32_hash(syn.autsk[n] ^ PTR_TO_INT(pos[n]->act));
	}
    }

    virtual void
    a_first(int n)
    {
      if (pos[n] != nodes[n]->out.end())
	hk ^= knuth32_hash(syn.autsk[n] ^ PTR_TO_INT(pos[n]->act));
      pos[n] = nodes[n]->out.begin();
      if (pos[n] != nodes[n]->out.end())
	hk ^= knuth32_hash(syn.autsk[n] ^ PTR_TO_INT(pos[n]->act));
    }

    virtual void
    a_next(int n)
    {
      if (pos[n] != nodes[n]->out.end())
	hk ^= knuth32_hash(syn.autsk[n] ^ PTR_TO_INT(pos[n]->act));
      ++(pos[n]);
      if (pos[n] != nodes[n]->out.end())
	hk ^= knuth32_hash(syn.autsk[n] ^ PTR_TO_INT(pos[n]->act));
    }

    virtual bool
    a_last(int n) const
    {
      return nodes[n]->out.end() == pos[n];
    }

    virtual void
    first()
    {
      loopless_modular_mixed_radix_gray_code::first();
      step_();
    }

    virtual void
    next()
    {
      loopless_modular_mixed_radix_gray_code::next();
      step_();
    }

    virtual bool
    done() const
    {
      return loopless_modular_mixed_radix_gray_code::done();
    }

    void
    step_()
    {
      while (!loopless_modular_mixed_radix_gray_code::done())
	{
	  std::cerr << "succ " << this << " considering transition #"
		    << hk; // << ":";
	  // for (unsigned n = 0; n < size; ++n)
	  // std::cerr << " " << pos[n]->act;
	  typedef sync::action_map::const_iterator it;
	  std::pair<it, it> p = syn.actions.equal_range(hk);
	  for (it i = p.first; i != p.second; ++i)
	    {
	      unsigned n;
	      for (n = 0; n < size; ++n)
		if (pos[n] !=  nodes[n]->out.end()
		    && i->second[n] != pos[n]->act)
		  break;
	      if (n == size)
		{
		  std::cerr << " -- GOOD" << std::endl;
		  return;
		}
	    }
	  std::cerr << " -- IGNORED" << std::endl;
	  loopless_modular_mixed_radix_gray_code::next();
	}
    }

    virtual state*
    current_state() const
    {
      sync::vnodes v(size);
      for (unsigned n = 0; n < size; ++n)
	if (pos[n] != nodes[n]->out.end())
	  v[n] = pos[n]->dst;
	else
	  v[n] = nodes[n];
      return new sync_state(syn.heap->insert(v));
    }

    virtual bdd
    current_condition() const
    {
      return bddtrue;
    }

    virtual bdd
    current_acceptance_conditions() const
    {
      return bddfalse;
    }
  };

  tgba_succ_iterator*
  sync::succ_iter(const state* s, const state*, const tgba*) const
  {
    const sync_state* s_ = dynamic_cast<const sync_state*>(s);
    const vnodes& n = s_->s->nodes;
    tgba_succ_iterator* i = new sync_transitions(n, *this);
    std::cerr << "sync " << this << " creates succ " << i
	      << " for state " << s_ << " (" << format_state(n)
	      << ")" << std::endl;
    return i;
  }

}
