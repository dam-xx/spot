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
    : data_(fact.get_core_data()), dict_(fact.get_dict()), init_(init)
  {
  }

  tgba_bdd_concrete::~tgba_bdd_concrete()
  {
  }

  void
  tgba_bdd_concrete::set_init_state(bdd s)
  {
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
