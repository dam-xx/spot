#ifndef SPOT_TGBA_TGBA_HH
# define SPOT_TGBA_TGBA_HH

#include "statebdd.hh"
#include "succiter.hh"
#include "tgbabdddict.hh"

namespace spot
{
  class tgba
  {
  public:
    virtual
    ~tgba()
    {
    }

    virtual state_bdd get_init_state() const = 0;
    virtual tgba_succ_iterator* succ_iter(state_bdd state) const = 0;

    virtual const tgba_bdd_dict& get_dict() const = 0;
  };

}

#endif // SPOT_TGBA_TGBA_HH
