#ifndef SPOT_LTLVISIT_DOTTY_HH
# define SPOT_LTLVISIT_DOTTY_HH

#include <ltlast/formula.hh>
#include <iostream>

namespace spot
{
  namespace ltl
  {
    /// \brief Write a formula tree using dot's syntax.
    /// \param f The formula to translate.
    /// \param os The stream where it should be output.
    ///
    /// \c dot is part of the GraphViz package
    /// http://www.research.att.com/sw/tools/graphviz/
    void dotty(const formula* f, std::ostream& os);
  }
}

#endif // SPOT_LTLVISIT_DOTTY_HH
