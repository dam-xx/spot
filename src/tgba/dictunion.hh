#ifndef SPOT_TGBA_DICTUNION_HH
# define SPOT_TGBA_DICTUNION_HH

#include "tgbabdddict.hh"

namespace spot
{
  tgba_bdd_dict
  tgba_bdd_dict_union(const tgba_bdd_dict& l, const tgba_bdd_dict& r);
}

#endif // SPOT_TGBA_DICTUNION_HH
