#ifndef SPOT_LTLVISIT_AST2STRING_HH
# define SPOT_LTLVISIT_AST2STRING_HH

#include <ltlast/formula.hh>
#include <iostream>

namespace spot 
{
  namespace ltl
  {
    void to_string(const formula& f, std::ostream& os);
  }
}

#endif // SPOT_LTLVISIT_AST2STRING_HH
