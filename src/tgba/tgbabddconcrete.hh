#ifndef SPOT_TGBA_TGBABDDCONCRETE_HH
# define SPOT_TGBA_TGBABDDCONCRETE_HH

#include "tgba.hh"
#include "statebdd.hh"
#include "tgbabddfactory.hh"
#include "succiterconcrete.hh"

namespace spot
{
  class tgba_bdd_concrete: public tgba
  {
  public:
    tgba_bdd_concrete(const tgba_bdd_factory& fact);
    tgba_bdd_concrete(const tgba_bdd_factory& fact, bdd init);
    ~tgba_bdd_concrete();

    void set_init_state(bdd s);
    state_bdd* get_init_state() const;
    bdd get_init_bdd() const;

    tgba_succ_iterator_concrete* succ_iter(const state* state) const;

    const tgba_bdd_dict& get_dict() const;
    const tgba_bdd_core_data& get_core_data() const;

  protected:
    tgba_bdd_core_data data_;
    tgba_bdd_dict dict_;
    bdd init_;
    friend class tgba_tgba_succ_iterator;
  };
}

#endif // SPOT_TGBA_TGBABDDCONCRETE_HH
