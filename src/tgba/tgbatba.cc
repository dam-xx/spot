#include <cassert>
#include "tgbatba.hh"
#include "bddprint.hh"
#include "ltlast/constant.hh"

namespace spot
{

  /// \brief A state for spot::tgba_tba_proxy.
  ///
  /// This state is in fact a pair of state: the state from the tgba
  /// automaton, and the "counter" (we use the accepting set
  /// BDD variable instead of an integer counter).
  class state_tba_proxy : public state
  {
  public:
    state_tba_proxy(state* s, bdd acc)
      :	s_(s), acc_(acc)
    {
    }

    /// Copy constructor
    state_tba_proxy(const state_tba_proxy& o)
      : state(),
	s_(o.real_state()->clone()),
	acc_(o.accepting_cond())
    {
    }

    virtual 
    ~state_tba_proxy()
    {
      delete s_;
    }

    state*
    real_state() const
    {
      return s_;
    }

    bdd
    accepting_cond() const
    {
      return acc_;
    }

    virtual int 
    compare(const state* other) const
    {
      const state_tba_proxy* o = dynamic_cast<const state_tba_proxy*>(other);
      assert(o);
      int res = s_->compare(o->real_state());
      if (res != 0)
	return res;
      return acc_.id() - o->accepting_cond().id();
    }
    
    virtual 
    state_tba_proxy* clone() const
    {
      return new state_tba_proxy(*this);
    }

  private:
    state* s_;
    bdd acc_;
  };


  /// \brief Iterate over the successors of tgba_tba_proxy computed on the fly.
  class tgba_tba_proxy_succ_iterator: public tgba_succ_iterator
  {
  public:
    tgba_tba_proxy_succ_iterator(tgba_succ_iterator* it,
				 bdd acc, bdd next_acc,
				 bdd the_accepting_cond)
      : it_(it), acc_(acc), next_acc_(next_acc), 
	the_accepting_cond_(the_accepting_cond)
    {
    }

    virtual
    ~tgba_tba_proxy_succ_iterator()
    {
    }

    // iteration

    void 
    first()
    {
      it_->first();
    }

    void 
    next()
    {
      it_->next();
    }

    bool 
    done() const
    {
      return it_->done();
    }

    // inspection

    state_tba_proxy* 
    current_state() const
    {
      state* s = it_->current_state();
      bdd acc;
      // Transition in the ACC_ accepting set should be directed
      // to the NEXT_ACC_ accepting set.
      if (acc_ == bddtrue 
	  || (acc_ & it_->current_accepting_conditions()) == acc_)
	acc = next_acc_;
      else
	acc = acc_;
      return new state_tba_proxy(s, acc);
    }

    bdd 
    current_condition() const
    {
      return it_->current_condition();
    }

    bdd 
    current_accepting_conditions() const
    {
      return the_accepting_cond_;
    }

  protected:
    tgba_succ_iterator* it_;
    bdd acc_;
    bdd next_acc_;
    bdd the_accepting_cond_;
  };


  tgba_tba_proxy::tgba_tba_proxy(const tgba* a)
    : a_(a)
  {
    bdd all = a_->all_accepting_conditions();

    // We will use one accepting condition for this automata.
    // Let's call it Acc[True].
    int v = get_dict()
      ->register_accepting_variable(ltl::constant::true_instance(), this);
    the_accepting_cond_ = bdd_ithvar(v);

    // Now build the "cycle" of accepting conditions.

    bdd last = bdd_satone(all);
    all &= !last;

    acc_cycle_[bddtrue] = last;

    while (all != bddfalse)
      {
	bdd next = bdd_satone(all);
	all &= !next;
	acc_cycle_[last] = next;
	last = next;
      }

    acc_cycle_[last] = bddtrue;
  }

  tgba_tba_proxy::~tgba_tba_proxy()
  {
    get_dict()->unregister_all_my_variables(this);
  }

  state* 
  tgba_tba_proxy::get_init_state() const
  {
    cycle_map::const_iterator i = acc_cycle_.find(bddtrue);
    assert(i != acc_cycle_.end());
    return new state_tba_proxy(a_->get_init_state(), i->second);
  }

  tgba_succ_iterator*
  tgba_tba_proxy::succ_iter(const state* local_state,
			    const state* global_state,
			    const tgba* global_automaton) const
  {
    const state_tba_proxy* s = 
      dynamic_cast<const state_tba_proxy*>(local_state);
    assert(s);

    tgba_succ_iterator* it = a_->succ_iter(s->real_state(), 
					   global_state, global_automaton);
    bdd acc = s->accepting_cond();
    cycle_map::const_iterator i = acc_cycle_.find(acc);
    assert(i != acc_cycle_.end());
    return 
      new tgba_tba_proxy_succ_iterator(it, acc, i->second,
				       (acc == bddtrue) 
				       ? the_accepting_cond_ : bddfalse);
  }
  
  bdd_dict* 
  tgba_tba_proxy::get_dict() const
  {
    return a_->get_dict();
  }
  
  std::string 
  tgba_tba_proxy::format_state(const state* state) const
  {
    const state_tba_proxy* s = 
      dynamic_cast<const state_tba_proxy*>(state);
    assert(s);
    return a_->format_state(s->real_state()) + "(" 
      + bdd_format_set(get_dict(), s->accepting_cond()) + ")";
  }
  
  bdd 
  tgba_tba_proxy::all_accepting_conditions() const
  {
    return the_accepting_cond_;
  }
  
  bdd 
  tgba_tba_proxy::neg_accepting_conditions() const
  {
    return !the_accepting_cond_;
  }
  
  bdd 
  tgba_tba_proxy::compute_support_conditions(const state* state) const
  {
    const state_tba_proxy* s = 
      dynamic_cast<const state_tba_proxy*>(state);
    assert(s);
    return a_->support_conditions(s->real_state());
  }
  
  bdd 
  tgba_tba_proxy::compute_support_variables(const state* state) const
  {
    const state_tba_proxy* s = 
      dynamic_cast<const state_tba_proxy*>(state);
    assert(s);
    return a_->support_variables(s->real_state());
  }

}
