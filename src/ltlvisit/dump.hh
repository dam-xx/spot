#ifndef SPOT_LTLVISIT_DUMP_HH
# define SPOT_LTLVISIT_DUMP_HH

#include <ltlast/formula.hh>
#include <iostream>

namespace spot 
{
  namespace ltl
  {
    void dump(const formula& f, std::ostream& os);
  }
}

#endif // SPOT_LTLVISIT_DUMP_HH
