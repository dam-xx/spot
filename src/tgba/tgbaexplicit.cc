#include "ltlast/atomic_prop.hh"
#include "ltlast/constant.hh"
#include "ltlvisit/destroy.hh"
#include "tgbaexplicit.hh"
#include <cassert>

namespace spot
{

  ////////////////////////////////////////
  // tgba_explicit_succ_iterator

  tgba_explicit_succ_iterator::tgba_explicit_succ_iterator
  (const tgba_explicit::state* s, bdd all_acc)
    : s_(s), all_accepting_conditions_(all_acc)
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
  tgba_explicit_succ_iterator::current_accepting_conditions()
  {
    return (*i_)->accepting_conditions & all_accepting_conditions_;
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

  state_explicit*
  state_explicit::clone() const
  {
    return new state_explicit(*this);
  }

  ////////////////////////////////////////
  // tgba_explicit


  tgba_explicit::tgba_explicit()
    : init_(0), all_accepting_conditions_(bddfalse),
      neg_accepting_conditions_(bddtrue),
      all_accepting_conditions_computed_(false)
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
    t->accepting_conditions = bddfalse;
    s->push_back(t);
    return t;
  }

  bdd
  tgba_explicit::get_condition(ltl::formula* f)
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
	ltl::destroy(f);
	v = i->second;
      }
    return ithvar(v);
  }

  void
  tgba_explicit::add_condition(transition* t, ltl::formula* f)
  {
    t->condition &= get_condition(f);
  }

  void
  tgba_explicit::add_neg_condition(transition* t, ltl::formula* f)
  {
    t->condition &= ! get_condition(f);
  }

  void
  tgba_explicit::declare_accepting_condition(ltl::formula* f)
  {
    tgba_bdd_dict::fv_map::iterator i = dict_.acc_map.find(f);
    if (i == dict_.acc_map.end())
      {
	int v;
	v = create_node();
	dict_.acc_map[f] = v;
	dict_.acc_formula_map[v] = f;
	neg_accepting_conditions_ &= !ithvar(v);
      }
  }

  bool
  tgba_explicit::has_accepting_condition(ltl::formula* f) const
  {
    tgba_bdd_dict::fv_map::const_iterator i = dict_.acc_map.find(f);
    return i != dict_.acc_map.end();
  }

  bdd
  tgba_explicit::get_accepting_condition(ltl::formula* f)
  {
    ltl::constant* c = dynamic_cast<ltl::constant*>(f);
    if (c)
      {
	switch (c->val())
	  {
	  case ltl::constant::True:
	    return bddtrue;
	  case ltl::constant::False:
	    return bddfalse;
	  }
	/* Unreachable code.  */
	assert(0);
      }

    tgba_bdd_dict::fv_map::iterator i = dict_.acc_map.find(f);
    assert (i != dict_.acc_map.end());
    ltl::destroy(f);
    bdd v = ithvar(i->second);
    v &= bdd_exist(neg_accepting_conditions_, v);
    return v;
  }

  void
  tgba_explicit::add_accepting_condition(transition* t, ltl::formula* f)
  {
    bdd c = get_accepting_condition(f);
    t->accepting_conditions |= c;
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
    return new tgba_explicit_succ_iterator(s->get_state(),
					   all_accepting_conditions());
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

  bdd
  tgba_explicit::all_accepting_conditions() const
  {
    if (!all_accepting_conditions_computed_)
      {
	bdd all = bddfalse;
	tgba_bdd_dict::fv_map::const_iterator i;
	for (i = dict_.acc_map.begin(); i != dict_.acc_map.end(); ++i)
	  {
	    bdd v = ithvar(i->second);
	    all |= v & bdd_exist(neg_accepting_conditions_, v);
	  }
	all_accepting_conditions_ = all;
	all_accepting_conditions_computed_ = true;
      }
    return all_accepting_conditions_;
  }

  bdd
  tgba_explicit::neg_accepting_conditions() const
  {
    return neg_accepting_conditions_;
  }

}
