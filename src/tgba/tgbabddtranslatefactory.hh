#ifndef SPOT_TGBA_TGBABDDTRANSLATEFACTORY_HH
# define SPOT_TGBA_TGBABDDTRANSLATEFACTORY_HH

#include "tgbabddfactory.hh"
#include "tgbabddconcrete.hh"

namespace spot
{

  class tgba_bdd_translate_factory: public tgba_bdd_factory
  {
  public:
    tgba_bdd_translate_factory(const tgba_bdd_concrete& from,
			       const tgba_bdd_dict& to);

    virtual ~tgba_bdd_translate_factory();

    bddPair* compute_pairs(const tgba_bdd_dict& from);
  
    const tgba_bdd_core_data& get_core_data() const;
    const tgba_bdd_dict& get_dict() const;
  
    bdd get_init_state() const;
  
  private:
    tgba_bdd_core_data data_;
    tgba_bdd_dict dict_;
    bdd init_;  
  };
  
  tgba_bdd_concrete defrag(const tgba_bdd_concrete& a);
}

#endif // SPOT_TGBA_TGBABDDTRANSLATEFACTORY_HH
