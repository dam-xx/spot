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
#include <algorithm>
#include <sstream>
#include "misc/hash.hh"
#include "misc/hashfunc.hh"
#include "misc/modgray.hh"
#include "ltlvisit/postfix.hh"
#include "ltlast/atomic_prop.hh"


#define PTR_TO_INT(p) \
  (reinterpret_cast<const char*>(p) - static_cast<const char*>(0))

namespace spot
{
  struct sync_transition
  {
    typedef std::vector<const saut::transition*> transitionv;
    transitionv trv;
    size_t h;
    sync_transition(unsigned n) : trv(n) {};

    size_t
    recompute_hash(const sync& syn) const
    {
      size_t hk = 0;
      for (unsigned j = 0; j < syn.size(); ++j)
	if (trv[j])
	  hk ^= knuth32_hash(syn.autsk[j] ^ PTR_TO_INT(trv[j]));
      return hk;
    }
  };

  struct sync_transition_hash
  {
    size_t
    operator()(const sync_transition* t) const
    {
      return t->h;
    }
  };

  struct sync_transition_eq
  {
    bool
    operator()(const sync_transition* a, const sync_transition* b) const
    {
      return (a->h != b->h) && (a->trv != b->trv);
    }
  };

  struct sync_transition_cmp
  {
    int
    operator()(const sync_transition* a, const sync_transition* b) const
    {
      if (a->h < b->h)
	return -1;
      if (a->h > b->h)
	return 1;
      sync_transition::transitionv::const_iterator i = a->trv.begin();
      sync_transition::transitionv::const_iterator j = b->trv.begin();

      for (; i != a->trv.end(); ++i, ++j)
	{
	  assert(j != b->trv.end());
	  if (*i < *j)
	    return -1;
	  if (*i > *j)
	    return 1;
	}
      assert(j == b->trv.end());
      return 0;
    }
  };

  struct sync_transition_set
  {
    typedef std::set<const sync_transition*, sync_transition_cmp> trset;
    trset s;
    bool
    has(const sync_transition* t) const
    {
      trset::const_iterator i = s.find(t);
      return i != s.end();
    }

    const sync_transition*
    insert(const sync_transition* t)
    {
      std::cerr << "Inserting transition " << t << " (#" << t->h << ") in "
		<< &s;
      std::pair<trset::iterator, bool> res = s.insert(t);
      std::cerr << " => " << *res.first  << std::endl;
      return *res.first;
    }

    const sync_transition*
    insert_or_delete(const sync_transition* t)
    {
      const sync_transition* u = insert(t);
      if (u != t)
	{
	  std::cerr << "Deleting transition " << t << std::endl;
	  delete t;
	}
      return u;
    }

    void
    intersection_update(const sync_transition_set* other)
    {
      trset res;
      set_intersection(s.begin(), s.end(),
		       other->s.begin(), other->s.end(),
		       std::inserter(res, res.begin()));
      s = res;
    }

    void
    free_not_in(const sync_transition_set& other)
    {
      for (trset::iterator i = s.begin(); i != s.end(); ++i)
	if (! other.has(*i))
	  delete *i;
    }

    void
    free_all()
    {
      for (trset::iterator i = s.begin(); i != s.end(); ++i)
	delete *i;
    }

    const sync_transition*
    pick_one()
    {
      assert(!s.empty());
      const sync_transition* t = *s.begin();
      s.erase(s.begin());
      return t;
    }

    bool
    empty()
    {
      return s.empty();
    }

    void
    delete_all()
    {
      for (trset::iterator i = s.begin(); i != s.end(); ++i)
	delete *i;
    }
  };

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

    const saut::node*
    operator[](unsigned n) const
    {
      return s->nodes[n];
    }
  };


  sync::sync(saut_list& sautlist, bool stubborn)
    : auts(sautlist.begin(), sautlist.end()),
      autsk(auts.size()),
      autssize(auts.size()),
      heap(new sync_state_heap),
      actions_back(auts.size()),
      stubborn(stubborn),
      aphi(bddtrue)
  {
    assert(!sautlist.empty());
    dict = sautlist.front()->get_dict();

    std::cerr << "sync " << this << " created for (";
    for (unsigned n = 0;;)
      {
	assert(auts[n]->get_dict() == dict);
	autsk[n] = knuth32_hash(PTR_TO_INT(auts[n] + n));
	std::cerr << auts[n];
	if (++n == autssize)
	  break;
	std::cerr << ", ";
      }
    std::cerr << ") with heap " << heap << std::endl;
  }

  sync::~sync()
  {
    delete heap;
  }


  void
  sync::set_stubborn(bool val)
  {
    stubborn = val;
    std::cerr << "sync " << this << " set_stubborn(" << val << ")" << std::endl;
  }

  void
  sync::set_aphi(bdd aphi)
  {
    this->aphi = aphi;
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
	if (++j >= autssize)
	  break;
	++i;
	std::cerr << ", ";
      }
    std::cerr << ")";
    if (not_epsilon)
      {
	std::cerr << " as #" << hk;

	const action_vect* p =
	  &(actions.insert(action_map::value_type(hk, v)))->second;

	for (unsigned j = 0; j < autssize; ++j)
	  actions_back[j][v[j]].push_front(p);
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
      assert (pos[n] != nodes[n]->out.end());
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
      bdd c = bddtrue;
      for (unsigned n = 0; n < size; ++n)
	if (pos[n] != nodes[n]->out.end())
	  c &= pos[n]->dst->prop_cond;
	else
	  c &= nodes[n]->prop_cond;
      return c;
    }

    bdd
    current_delta() const
    {
      bdd src = bddtrue;
      bdd dst = bddtrue;
      for (unsigned n = 0; n < size; ++n)
	if (pos[n] != nodes[n]->out.end())
	  {
	    src &= pos[n]->src->prop_cond;
	    dst &= pos[n]->src->prop_cond;
	  }
      return bdd_exist(src, dst) & bdd_exist(dst, src);
    }

    virtual bdd
    current_acceptance_conditions() const
    {
      return bddfalse;
    }

    sync_transition*
    current_transition() const
    {
      sync_transition* res = new sync_transition(size);
      size_t h = 0;
      std::cerr << "current_transition " << res << " = [";
      for (unsigned n = 0; n < size; ++n)
	{
	  const saut::transition* t;
	  if (pos[n] != nodes[n]->out.end())
	    t = &*pos[n];
	  else
	    t = 0;
	  res->trv[n] = t;
	  if (t)
	    {
	      std::cerr << "(" << t->src << ", "
			<< t->act << ", " << t->dst << ")";
	      h ^= knuth32_hash(syn.autsk[n] ^ PTR_TO_INT(t));
	    }
	  else
	    {
	      std::cerr << "(.)";
	    }
	}
      res->h = h;
      std::cerr << "] #" << h << std::endl;;
      assert(h == res->recompute_hash(syn));
      return res;
    }

    std::string
    current_annotation() const
    {
      std::ostringstream os;
      for (unsigned n = 0;;)
	{
	  if (pos[n] != nodes[n]->out.end())
	    os << *pos[n]->act->name;
	  else
	    os << ".";
	  if (++n == size)
	    break;
	  os << ",";
	}
      return os.str();
    }
  };

  class sync_transition_set_iterator : public tgba_succ_iterator
  {
    const sync& syn;
    sync_transition_set::trset set;
    sync_transition_set::trset::const_iterator pos;
    const sync::vnodes& n;

  public:
    sync_transition_set_iterator(const sync& syn,
				 sync_transition_set set,
				 const sync::vnodes& n)
      : syn(syn), set(set.s), n(n)
    {
      std::cerr << "sync_transition_set_iterator " << this << " size "
		<< this->set.size() << std::endl;
    }

    ~sync_transition_set_iterator()
    {
      for (sync_transition_set::trset::iterator i = set.begin();
	   i != set.end(); ++i)
	delete *i;
    }

    virtual void
    first()
    {
      pos = set.begin();
    }

    virtual void
    next()
    {
      ++pos;
    }

    virtual bool
    done() const
    {
      return pos == set.end();
    }

    virtual state*
    current_state() const
    {
      sync::vnodes v(syn.size());
      for (unsigned i = 0; i < syn.size(); ++i)
	if ((*pos)->trv[i])
	  v[i] = (*pos)->trv[i]->dst;
	else
	  v[i] = n[i];
      return new sync_state(syn.heap->insert(v));
    }

    virtual bdd
    current_condition() const
    {
      bdd c = bddtrue;
      for (unsigned i = 0; i < syn.size(); ++i)
	if ((*pos)->trv[i])
	  c &= (*pos)->trv[i]->dst->prop_cond;
	else
	  c &= n[i]->prop_cond;
      return c;
    }

    virtual bdd
    current_acceptance_conditions() const
    {
      return bddfalse;
    }

    std::string
    current_annotation() const
    {
      std::ostringstream os;
      for (unsigned n = 0;;)
	{
	  if ((*pos)->trv[n])
	    os << *(*pos)->trv[n]->act->name;
	  else
	    os << ".";
	  if (++n == syn.size())
	    break;
	  os << ",";
	}
      return os.str();
    }
  };


  unsigned
  sync::is_active(const sync_state* q, const sync_transition* t) const
  {
    unsigned n = size();
    const sync_transition::transitionv& trv = t->trv;
    for (unsigned i = 0; i < n; ++i)
      {
	if (trv[i] && (*q)[i] != trv[i]->src)
	  return i;
      }
    return n;
  }

  struct sync_part : public loopless_modular_mixed_radix_gray_code
  {
    const sync::action_vect& v;
    std::vector<saut::transitionsp_list::const_iterator> pos;
    size_t hk;
    const sync& syn;
    unsigned size;
    sync_part(const sync& syn, const sync::action_vect* v_)
      : loopless_modular_mixed_radix_gray_code(syn.size()),
	v(*v_),
	pos(syn.size()),
	syn(syn),
	size(syn.size())
    {
      hk = 0;
      for (unsigned n = 0; n < size; ++n)
	{
	  if (v[n])
	    {
	      pos[n] = v[n]->tia.begin();
	      assert(pos[n] != v[n]->tia.end());
	      hk ^= knuth32_hash(syn.autsk[n] ^ PTR_TO_INT(*pos[n]));
	    }
	}
    }

    virtual void
    a_first(int j)
    {
      if (v[j])
	{
	  if (pos[j] != v[j]->tia.end())
	    hk ^= knuth32_hash(syn.autsk[j] ^ PTR_TO_INT(*pos[j]));
	  pos[j] = v[j]->tia.begin();
	  hk ^= knuth32_hash(syn.autsk[j] ^ PTR_TO_INT(*pos[j]));
	}
    }

    virtual void
    a_next(int j)
    {
      if (v[j])
	{
	  hk ^= knuth32_hash(syn.autsk[j] ^ PTR_TO_INT(*pos[j]));
	  ++pos[j];
	  if (pos[j] != v[j]->tia.end())
	    hk ^= knuth32_hash(syn.autsk[j] ^ PTR_TO_INT(*pos[j]));
	}
    }

    virtual bool
    a_last(int j) const
    {
      if (v[j])
	{
	  assert(!v[j]->tia.empty());
	  return &*pos[j] == &*v[j]->tia.rbegin();
	}
      return true;
    }

    template <class T>
    void
    run(T i)
    {
      first();
      while (!done())
	{
	  sync_transition* t = new sync_transition(size);
	  t->h = hk;
	  for (unsigned n = 0; n < size; ++n)
	    if (v[n])
	      t->trv[n] = *pos[n];
	    else
	      t->trv[n] = 0;
	  assert(hk == t->recompute_hash(syn));
	  *i++ = t;
	  next();
	}
    }

    template <class T>
    void
    run_restricted(T i, unsigned n, const saut::node* s)
    {
      assert(v[n]);
      first();
      while (!done())
	{
	  // Could be done a lot faster by restricting the ranges
	  // of modgray.
	  if ((*pos[n])->dst == s)
	    {
	      sync_transition* t = new sync_transition(size);
	      t->h = hk;
	      for (unsigned n = 0; n < size; ++n)
		if (v[n])
		  t->trv[n] = *pos[n];
		else
		  t->trv[n] = 0;
	      assert(hk == t->recompute_hash(syn));
	      *i++ = t;
	      next();
	    }
	}
    }

  };

  sync::stlist
  sync::E1UE2(const sync_transition* t) const
  {
    stlist res;
    for (unsigned i = 0; i < size(); ++i)
      {
	const saut::transition* ti = t->trv[i];
	if (ti)
	  {
	    typedef std::set<const saut::action*> con_set;
	    con_set con;
	    if (ti->src != ti->dst)
	      {
		const saut::transitions_list& out = ti->src->out;
		for (saut::transitions_list::const_iterator it = out.begin();
		     it != out.end(); ++it)
		  con.insert(it->act);
	      }
	    else
	      {
		const saut::transitions_list& out = ti->src->out;
		for (saut::transitions_list::const_iterator it = out.begin();
		     it != out.end(); ++it)
		  if (it->src != it->dst)
		    con.insert(it->act);
	      }

	    for (con_set::const_iterator c = con.begin();
		 c != con.end(); ++c)
	      {
		action_back_map::const_iterator j = actions_back[i].find(*c);
		assert (j != actions_back[i].end());
		const action_vect_list& l = j->second;
		for (action_vect_list::const_iterator k = l.begin();
		     k != l.end(); ++k)
		  {
		    sync_part s(*this, *k);
		    s.run(inserter(res, res.begin()));
		  }
	      }
	  }
      }
    return res;
  }

  sync::stlist
  sync::E3(const sync_transition* t, unsigned i) const
  {
    stlist res;
    std::cerr << t << "/" << i << " = (" << t->trv[i]->src << ", "
	      << t->trv[i]->act << ", " << t->trv[i]->dst << ")"
	      << std::endl;
    const saut::node* q_i = t->trv[i]->src;
    for (saut::transitionsp_list::const_iterator p = q_i->in.begin();
	 p != q_i->in.end(); ++p)
      {
	action_back_map::const_iterator j = actions_back[i].find((*p)->act);
	assert (j != actions_back[i].end());
	const action_vect_list& l = j->second;
	for (action_vect_list::const_iterator k = l.begin();
	     k != l.end(); ++k)
	  {
	    sync_part s(*this, *k);
	    s.run_restricted(inserter(res, res.begin()), i, q_i);
	  }

      }
    return res;
  }

  bdd
  delta(const sync_transition* t, unsigned size)
  {
    bdd src = bddtrue;
    bdd dst = bddtrue;
    for (unsigned n = 0; n < size; ++n)
      if (t->trv[n])
	{
	  src &= t->trv[n]->src->prop_list;
	  dst &= t->trv[n]->dst->prop_list;
	}

    return bdd_exist(src, dst) & bdd_exist(dst, src);
  }


  sync_transition_set
  stubborn_set_trans(const sync* syn,
		     const sync_state* q,
		     const sync_transition* n,
		     bdd aphi)
  {
    unsigned size = syn->size();
    sync_transition_set done;
    sync_transition_set actives;
    sync_transition_set todo;
    todo.insert(n);
    done.insert(n);
    while (!todo.empty())
      {
	const sync_transition* t = todo.pick_one();
	std::cerr << "picking transition " << t
		  << " from " << &todo << std::endl;
	unsigned which = syn->is_active(q, t);
	sync::stlist toadd;
	if (which < size)
	  {
	    std::cerr << "transition " << t << " is inactive (" << which << ")"
		      << std::endl;
	    toadd = syn->E3(t, which);
	  }
	else
	  {
	    std::cerr << "transition " << t << " is active" << std::endl;
	    toadd = syn->E1UE2(t);
	    actives.insert(t);
	  }
	for (sync::stlist::const_iterator i = toadd.begin();
	     i != toadd.end(); ++i)
	  {
	    if (done.insert_or_delete(*i) == *i)
	      {
		bdd d = delta(*i, size);
		if (bdd_exist(d, aphi) != d)
		  {
		    done.free_all();
		    while (++i != toadd.end())
		      delete *i;
		    return sync_transition_set();
		  }
		todo.insert(*i);
	      }
	  }
      }
    done.free_not_in(actives);
    return actives;
  }

  tgba_succ_iterator*
  sync::succ_iter(const state* s, const state*, const tgba*) const
  {
    const sync_state* s_ = dynamic_cast<const sync_state*>(s);
    const vnodes& n = s_->s->nodes;
    sync_transitions* i = new sync_transitions(n, *this);
    i->first();
    if (!stubborn || i->done())
      {
      nostubborn:
	std::cerr << "sync " << this << " creates succ " << i
		  << " for state " << s_ << " (" << format_state(n)
		  << ")" << std::endl;
	return i;
      }
    else
      {
	// Try to pick a non-observed transition.
	while (!i->done())
	  {
	    bdd delta = i->current_delta();
	    if (bdd_exist(delta, aphi) == delta)
	      break;
	    i->next();
	  }
	if (i->done())
	  goto nostubborn;

	sync_transition* t = i->current_transition();
	sync_transition_set set = stubborn_set_trans(this, s_, t, aphi);
	if (set.empty())
	  {
	    std::cerr << "sync " << this << " no stubborn set" << std::endl;
	    goto nostubborn;
	  }
	sync_transition_set_iterator* j =
	  new sync_transition_set_iterator(*this, set, s_->s->nodes);
	delete i;

	std::cerr << "sync " << this << " creates stubborn_succ " << j
		  << " for state " << s_ << " (" << format_state(n)
		  << ")" << std::endl;
	return j;
      }
  }

  namespace
  {
    struct bdd_prop_collector : public ltl::postfix_visitor
    {
      bdd_prop_collector(bdd_dict* d)
	: ltl::postfix_visitor(), d(d), result(bddtrue)
      {
      }

      virtual ~bdd_prop_collector() {}

      virtual void doit(spot::ltl::atomic_prop* ap)
      {
	result &= bdd_ithvar(d->var_map[ap]);
      }

      bdd_dict* d;
      bdd result;
    };
  }


  void
  sync::set_aphi(ltl::formula* f)
  {
    bdd_prop_collector v(dict);
    const_cast<ltl::formula*>(f)->accept(v);
    set_aphi(v.result);
  }

  std::string
  sync::transition_annotation(const tgba_succ_iterator* t) const
  {
    const sync_transitions* tr = dynamic_cast<const sync_transitions*>(t);
    if (tr)
      return tr->current_annotation();
    const sync_transition_set_iterator* trs =
      dynamic_cast<const sync_transition_set_iterator*>(t);
    assert(trs);
    return trs->current_annotation();
  }



}
