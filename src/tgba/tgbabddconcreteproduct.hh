#ifndef SPOT_TGBA_TGBABDDCONCRETEPRODUCT_HH
# define SPOT_TGBA_TGBABDDCONCRETEPRODUCT_HH

#include "tgbabddconcrete.hh"

namespace spot
{
  /// \brief Multiplies two tgba::tgba_bdd_concrete automata.
  ///
  /// This function build the resulting product, as another
  /// tgba::tgba_bdd_concrete automaton.
  tgba_bdd_concrete
  product(const tgba_bdd_concrete& left, const tgba_bdd_concrete& right);
}

#endif // SPOT_TGBA_TGBABDDCONCRETEPRODUCT_HH
