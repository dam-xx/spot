#ifndef SPOT_TGBA_STATEBDD_HH
# define SPOT_TGBA_STATEBDD_HH

#include <bdd.h>
#include "state.hh"

namespace spot
{
  class state_bdd: public state
  {
  public:
    state_bdd(bdd s)
      : state_(s)
    {
    }

    bdd 
    as_bdd() const
    {
      return state_;
    }
    
    virtual int compare(const state* other) const;

  protected:
    bdd state_;
  };
}

#endif // SPOT_TGBA_STATEBDD_HH
