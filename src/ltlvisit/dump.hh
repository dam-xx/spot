#ifndef SPOT_LTLVISIT_DUMP_HH
# define SPOT_LTLVISIT_DUMP_HH

#include "ltlast/formula.hh"
#include <iostream>

namespace spot
{
  namespace ltl
  {
    /// \brief Dump a formula tree.
    /// \param f The formula to dump.
    /// \param os The stream where it should be output.
    ///
    /// This is useful to display a formula when debugging.
    std::ostream& dump(const formula* f, std::ostream& os);
  }
}

#endif // SPOT_LTLVISIT_DUMP_HH
