#ifndef SPOT_TGBAALGOS_SAVE_HH
# define SPOT_TGBAALGOS_SAVE_HH

#include "tgba/tgba.hh"
#include <iostream>

namespace spot
{
  /// \brief Save reachable states in text format.
  std::ostream& tgba_save_reachable(std::ostream& os, const tgba* g);
}

#endif // SPOT_TGBAALGOS_SAVE_HH
