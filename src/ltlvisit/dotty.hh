#ifndef SPOT_LTLVISIT_DOTTY_HH
# define SPOT_LTLVISIT_DOTTY_HH

#include <ltlast/formula.hh>
#include <iostream>

namespace spot 
{
  namespace ltl
  {
    void dotty(const formula& f, std::ostream& os);
  }
}

#endif // SPOT_LTLVISIT_DOTTY_HH
