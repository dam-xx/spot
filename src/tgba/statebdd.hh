#ifndef SPOT_TGBA_STATEBDD_HH
# define SPOT_TGBA_STATEBDD_HH

#include <bdd.h>
#include "state.hh"

namespace spot
{
  /// A state whose representation is a BDD.
  class state_bdd: public state
  {
  public:
    state_bdd(bdd s)
      : state_(s)
    {
    }

    /// Return the BDD part of the state.
    bdd 
    as_bdd() const
    {
      return state_;
    }
    
    virtual int compare(const state* other) const;

  protected:
    bdd state_;			///< BDD representation of the state.
  };
}

#endif // SPOT_TGBA_STATEBDD_HH
