#include "ltlast/atomic_prop.hh"
#include "tgbaexplicit.hh"
#include <cassert>

namespace spot
{

  ////////////////////////////////////////
  // tgba_explicit_succ_iterator

  tgba_explicit_succ_iterator::tgba_explicit_succ_iterator
  (const tgba_explicit::state* s)
    : s_(s)
  {
  }

  void 
  tgba_explicit_succ_iterator::first()
  {
    i_ = s_->begin();
  }

  void 
  tgba_explicit_succ_iterator::next()
  {
    ++i_;
  }

  bool 
  tgba_explicit_succ_iterator::done()
  {
    return i_ == s_->end();
  }

  state_explicit* 
  tgba_explicit_succ_iterator::current_state()
  {
    return new state_explicit((*i_)->dest);
  }

  bdd 
  tgba_explicit_succ_iterator::current_condition()
  {
    return (*i_)->condition;
  }

  bdd 
  tgba_explicit_succ_iterator::current_promise()
  {
    return (*i_)->promise;
  }


  ////////////////////////////////////////
  // state_explicit

  const tgba_explicit::state* 
  state_explicit::get_state() const
  {
    return state_;
  }

  int
  state_explicit::compare(const spot::state* other) const
  {
    const state_explicit* o = dynamic_cast<const state_explicit*>(other);
    assert(o);
    return o->get_state() - get_state();
  }

  ////////////////////////////////////////
  // tgba_explicit


  tgba_explicit::tgba_explicit()
    : init_(0)
  {
  }

  tgba_explicit::~tgba_explicit()
  {
    ns_map::iterator i;
    for (i = name_state_map_.begin(); i != name_state_map_.end(); ++i)
      {
	tgba_explicit::state::iterator i2;
	for (i2 = i->second->begin(); i2 != i->second->end(); ++i2)
	  delete *i2;
	delete i->second;
      }
  }
  
  tgba_explicit::state*
  tgba_explicit::add_state(const std::string& name)
  {
    ns_map::iterator i = name_state_map_.find(name);
    if (i == name_state_map_.end())
      {
	tgba_explicit::state* s = new tgba_explicit::state;
	name_state_map_[name] = s;
	state_name_map_[s] = name;

	// The first state we add is the inititial state.
	if (! init_)
	  init_ = s;

	return s;
      }
    return i->second;
  }

  tgba_explicit::transition*
  tgba_explicit::create_transition(const std::string& source, 
				   const std::string& dest)
  {
    tgba_explicit::state* s = add_state(source);
    tgba_explicit::state* d = add_state(dest);
    transition* t = new transition;
    t->dest = d;
    t->condition = bddtrue;
    t->promise = bddtrue;
    s->push_back(t);
    return t;
  }

  int tgba_explicit::get_condition(ltl::formula* f)
  {
    assert(dynamic_cast<ltl::atomic_prop*>(f));
    tgba_bdd_dict::fv_map::iterator i = dict_.var_map.find(f);
    int v;
    if (i == dict_.var_map.end())
      {
	v = create_node();
	dict_.var_map[f] = v;
	dict_.var_formula_map[v] = f;
      }
    else
      {
	v = i->second;
      }
    return v;
  }

  void tgba_explicit::add_condition(transition* t, ltl::formula* f)
  {
    t->condition &= ithvar(get_condition(f));
  }

  void tgba_explicit::add_neg_condition(transition* t, ltl::formula* f)
  {
    t->condition &= ! ithvar(get_condition(f));
  }

  int tgba_explicit::get_promise(ltl::formula* f)
  {
    tgba_bdd_dict::fv_map::iterator i = dict_.prom_map.find(f);
    int v;
    if (i == dict_.prom_map.end())
      {
	v = create_node();
	dict_.prom_map[f] = v;
	dict_.prom_formula_map[v] = f;
      }
    else
      {
	v = i->second;
      }
    return v;
  }

  void tgba_explicit::add_promise(transition* t, ltl::formula* f)
  {
    t->promise &= ithvar(get_promise(f));
  }

  void tgba_explicit::add_neg_promise(transition* t, ltl::formula* f)
  {
    t->promise &= ! ithvar(get_promise(f));
  }

  state* 
  tgba_explicit::get_init_state() const
  {
    return new state_explicit(init_);
  }

  tgba_succ_iterator* 
  tgba_explicit::succ_iter(const spot::state* state) const
  {
    const state_explicit* s = dynamic_cast<const state_explicit*>(state);
    assert(s);
    return new tgba_explicit_succ_iterator(s->get_state());
  }
    
  const tgba_bdd_dict& 
  tgba_explicit::get_dict() const
  {
    return dict_;
  }

  std::string 
  tgba_explicit::format_state(const spot::state* s) const
  {
    const state_explicit* se = dynamic_cast<const state_explicit*>(s);
    assert(se);
    sn_map::const_iterator i = state_name_map_.find(se->get_state());
    assert(i != state_name_map_.end());
    return i->second;
  }

}
