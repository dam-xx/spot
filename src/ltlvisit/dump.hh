#ifndef SPOT_LTLVISIT_DUMP_HH
# define SPOT_LTLVISIT_DUMP_HH

#include <ltlast/formulae.hh>
#include <iostream>

namespace spot 
{
  namespace ltl
  {
    void dump(const formulae& f, std::ostream& os);
  }
}

#endif // SPOT_LTLVISIT_DUMP_HH
