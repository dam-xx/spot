#include "tgbabddconcrete.hh"

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

  state_bdd
  tgba_bdd_concrete::get_init_state() const
  {
    return init_;
  }

  tgba_succ_iterator_concrete*
  tgba_bdd_concrete::succ_iter(bdd state) const
  {
    bdd succ_set = bdd_replace(bdd_exist(data_.relation & state,
					 data_.now_set),
			       data_.next_to_now);
    return new tgba_succ_iterator_concrete(data_, succ_set);
  }

  tgba_succ_iterator_concrete*
  tgba_bdd_concrete::init_iter() const
  {
    return new tgba_succ_iterator_concrete(data_, init_);
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
