#ifndef SPOT_LTLVISIT_DOTTY_HH
# define SPOT_LTLVISIT_DOTTY_HH

#include <ltlast/formula.hh>
#include <iostream>

namespace spot
{
  namespace ltl
  {
    /// \brief Write a formula tree using dot's syntax.
    /// \param os The stream where it should be output.
    /// \param f The formula to translate.
    ///
    /// \c dot is part of the GraphViz package
    /// http://www.research.att.com/sw/tools/graphviz/
    std::ostream& dotty(std::ostream& os, const formula* f);
  }
}

#endif // SPOT_LTLVISIT_DOTTY_HH
