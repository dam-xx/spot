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
  tgba_explicit_succ_iterator::done() const
  {
    return i_ == s_->end();
  }

  state_explicit*
  tgba_explicit_succ_iterator::current_state() const
  {
    return new state_explicit((*i_)->dest);
  }

  bdd
  tgba_explicit_succ_iterator::current_condition() const
  {
    return (*i_)->condition;
  }

  bdd
  tgba_explicit_succ_iterator::current_accepting_conditions() const
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


  tgba_explicit::tgba_explicit(bdd_dict* dict)
    : dict_(dict), init_(0), all_accepting_conditions_(bddfalse),
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
    dict_->unregister_all_my_variables(this);
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
	// It can also be overridden with set_init_state().
	if (! init_)
	  init_ = s;

	return s;
      }
    return i->second;
  }

  void
  tgba_explicit::set_init_state(const std::string& state)
  {
    tgba_explicit::state* s = add_state(state);
    init_ = s;
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
    int v = dict_->register_proposition(f, this);
    ltl::destroy(f);
    return bdd_ithvar(v);
  }

  void
  tgba_explicit::add_condition(transition* t, ltl::formula* f)
  {
    t->condition &= get_condition(f);
  }

  void
  tgba_explicit::add_neg_condition(transition* t, ltl::formula* f)
  {
    t->condition -= get_condition(f);
  }

  void
  tgba_explicit::declare_accepting_condition(ltl::formula* f)
  {
    int v = dict_->register_accepting_variable(f, this);
    ltl::destroy(f);
    bdd neg = bdd_nithvar(v);
    neg_accepting_conditions_ &= neg;

    // Append neg to all acceptance conditions.
    ns_map::iterator i;
    for (i = name_state_map_.begin(); i != name_state_map_.end(); ++i)
      {
	tgba_explicit::state::iterator i2;
	for (i2 = i->second->begin(); i2 != i->second->end(); ++i2)
	  (*i2)->accepting_conditions &= neg;
      }

    all_accepting_conditions_computed_ = false;
  }

  void
  tgba_explicit::complement_all_accepting_conditions()
  {
    bdd all = all_accepting_conditions();
    ns_map::iterator i;
    for (i = name_state_map_.begin(); i != name_state_map_.end(); ++i)
      {
	tgba_explicit::state::iterator i2;
	for (i2 = i->second->begin(); i2 != i->second->end(); ++i2)
	  {
	    (*i2)->accepting_conditions = all - (*i2)->accepting_conditions;
	  }
      }
  }

  bool
  tgba_explicit::has_accepting_condition(ltl::formula* f) const
  {
    return dict_->is_registered_accepting_variable(f, this);
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
    bdd_dict::fv_map::iterator i = dict_->acc_map.find(f);
    assert(has_accepting_condition(f));
    /* If this second assert fails and the first doesn't,
       things are badly broken.  This has already happened. */
    assert(i != dict_->acc_map.end());
    ltl::destroy(f);
    bdd v = bdd_ithvar(i->second);
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
  tgba_explicit::succ_iter(const spot::state* state,
			   const spot::state* global_state,
			   const tgba* global_automaton) const
  {
    const state_explicit* s = dynamic_cast<const state_explicit*>(state);
    assert(s);
    (void) global_state;
    (void) global_automaton;
    return new tgba_explicit_succ_iterator(s->get_state(),
					   all_accepting_conditions());
  }

  bdd
  tgba_explicit::compute_support_conditions(const spot::state* in) const
  {
    const state_explicit* s = dynamic_cast<const state_explicit*>(in);
    assert(s);
    const state* st = s->get_state();

    bdd res = bddtrue;
    tgba_explicit::state::const_iterator i;
    for (i = st->begin(); i != st->end(); ++i)
      res |= (*i)->condition;
    return res;
  }

  bdd
  tgba_explicit::compute_support_variables(const spot::state* in) const
  {
    const state_explicit* s = dynamic_cast<const state_explicit*>(in);
    assert(s);
    const state* st = s->get_state();

    bdd res = bddtrue;
    tgba_explicit::state::const_iterator i;
    for (i = st->begin(); i != st->end(); ++i)
      res &= bdd_support((*i)->condition);
    return res;
  }

  bdd_dict*
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

	// Build all_accepting_conditions_ from neg_accepting_conditions_
	// I.e., transform !A & !B & !C into
	//        A & !B & !C
	//     + !A &  B & !C
	//     + !A & !B &  C
	bdd cur = neg_accepting_conditions_;
	while (cur != bddtrue)
	  {
	    assert(cur != bddfalse);

	    bdd v = bdd_ithvar(bdd_var(cur));
	    all |= v & bdd_exist(neg_accepting_conditions_, v);

	    assert(bdd_high(cur) != bddtrue);
	    cur = bdd_low(cur);
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
