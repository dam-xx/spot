#ifndef SPOT_LTLVISIT_AST2STRING_HH
# define SPOT_LTLVISIT_AST2STRING_HH

#include <ltlast/formula.hh>
#include <iostream>

namespace spot
{
  namespace ltl
  {
    std::ostream& to_string(const formula* f, std::ostream& os);
    std::string to_string(const formula* f);
  }
}

#endif // SPOT_LTLVISIT_AST2STRING_HH
