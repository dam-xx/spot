#include "tgbabddconcrete.hh"
#include "bddprint.hh"
#include <cassert>

namespace spot
{
  tgba_bdd_concrete::tgba_bdd_concrete(const tgba_bdd_factory& fact)
    : data_(fact.get_core_data()), dict_(fact.get_dict())
  {
  }

  tgba_bdd_concrete::tgba_bdd_concrete(const tgba_bdd_factory& fact, bdd init)
    : data_(fact.get_core_data()), dict_(fact.get_dict())
  {
    set_init_state(init);
  }

  tgba_bdd_concrete::~tgba_bdd_concrete()
  {
  }

  void
  tgba_bdd_concrete::set_init_state(bdd s)
  {
    // Usually, the ltl2tgba translator will return an
    // initial state which does not include all true Now variables,
    // even though the truth of some Now variables is garanteed.
    //
    // For instance, when building the automata for the formula GFa,
    // the translator will define the following two equivalences
    //    Now[Fa] <=> a | (Prom[a] & Next[Fa])
    //    Now[GFa] <=> Now[Fa] & Next[GFa]
    // and return Now[GFa] as initial state.
    //
    // Starting for state Now[GFa], we could then build
    // the following automaton:
    //    In state Now[GFa]:
    //        if `a', go to state Now[GFa]
    //        if True, go to state Now[GFa] & Now[Fa] with Prom[a]
    //    In state Now[GFa] & Now[Fa]:
    //        if `a', go to state Now[GFa]
    //        if True, go to state Now[GFa] & Now[Fa] with Prom[a]
    //
    // As we can see, states Now[GFa] and Now[GFa] & Now[Fa] share
    // the same actions.  This is no surprise, because
    // Now[GFa] <=> Now[GFa] & Now[Fa] according to the equivalences
    // defined by the translator.
    //
    // What sounds bogus is that we dissociated the two states:
    // there should be only one, so that the automaton become
    //
    //    In state Now[GFa] & Now[Fa]:
    //        if `a', go to state Now[GFa] & Now[GFa]
    //        if True, go to state Now[GFa] & Now[Fa] with Prom[a]
    //
    // To achieve this, we immerse the state into the relation
    // to collect the status of other Now variables.
    s &= bdd_relprod(s, data_.relation, data_.notnow_set);
    init_ = s;
  }

  state_bdd*
  tgba_bdd_concrete::get_init_state() const
  {
    return new state_bdd(init_);
  }

  bdd
  tgba_bdd_concrete::get_init_bdd() const
  {
    return init_;
  }

  tgba_succ_iterator_concrete*
  tgba_bdd_concrete::succ_iter(const state* state) const
  {
    const state_bdd* s = dynamic_cast<const state_bdd*>(state);
    assert(s);
    bdd succ_set = bdd_replace(bdd_exist(data_.relation & s->as_bdd(),
					 data_.now_set),
			       data_.next_to_now);

    // Immerse known Now variables into the relation to compute the
    // status of any other Now variables.  See the comment in
    // set_init_state for the rational.  Note that unlike
    // set_init_state, we work only from the Now variables, not from
    // any other atomic propositions around.  This is because the Now
    // variables and the atomic propositions do not correspond to the
    // same instant: atomic propositions describe what we must verify
    // now while Now variables describe the state where we go (and
    // where something else we have to be checked) -- actually these
    // Now variables are really Next variables renammed for
    // convenience.
    succ_set &= bdd_relprod(bdd_exist(succ_set, data_.notnow_set),
			    data_.relation, data_.notnow_set);

    return new tgba_succ_iterator_concrete(data_, succ_set);
  }

  std::string
  tgba_bdd_concrete::format_state(const state* state) const
  {
    const state_bdd* s = dynamic_cast<const state_bdd*>(state);
    assert(s);
    return bdd_format_set(dict_, s->as_bdd());
  }

  const tgba_bdd_dict&
  tgba_bdd_concrete::get_dict() const
  {
    return dict_;
  }

  const tgba_bdd_core_data&
  tgba_bdd_concrete::get_core_data() const
  {
    return data_;
  }

}
