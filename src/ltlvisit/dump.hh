#ifndef SPOT_LTLVISIT_DUMP_HH
# define SPOT_LTLVISIT_DUMP_HH

#include "ltlast/formula.hh"
#include <iostream>

namespace spot
{
  namespace ltl
  {
    /// \brief Dump a formula tree.
    /// \param os The stream where it should be output.
    /// \param f The formula to dump.
    ///
    /// This is useful to display a formula when debugging.
    std::ostream& dump(std::ostream& os, const formula* f);
  }
}

#endif // SPOT_LTLVISIT_DUMP_HH
