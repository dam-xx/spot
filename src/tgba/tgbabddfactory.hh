#ifndef SPOT_TGBA_TGBABDDFACTORY_H
# define SPOT_TGBA_TGBABDDFACTORY_H

#include "tgbabddcoredata.hh"
#include "tgbabdddict.hh"

namespace spot
{
  class tgba_bdd_factory
  {
  public:
    virtual const tgba_bdd_core_data& get_core_data() const = 0;
    virtual const tgba_bdd_dict& get_dict() const = 0;
  };
}

#endif // SPOT_TGBA_TGBABDDFACTORY_H
