#ifndef SPOT_TGBAALGOS_DOTTY_HH
# define SPOT_TGBAALGOS_DOTTY_HH

#include "tgba/tgba.hh"
#include <iostream>

namespace spot
{
  /// \brief Print reachable states in dot format.
  std::ostream& dotty_reachable(std::ostream& os, const tgba* g);
}

#endif // SPOT_TGBAALGOS_DOTTY_HH
