#ifndef SPOT_TGBA_LTL2TGBA_HH
# define SPOT_TGBA_LTL2TGBA_HH

#include "ltlast/formula.hh"
#include "tgbabddconcrete.hh"

namespace spot
{
  /// Build a spot::tgba_bdd_concrete from an LTL formula.
  tgba_bdd_concrete ltl_to_tgba(const ltl::formula* f);
}

#endif // SPOT_TGBA_LTL2TGBA_HH
