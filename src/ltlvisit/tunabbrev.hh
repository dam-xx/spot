#ifndef SPOT_LTLVISIT_TUNABBREV_HH
# define SPOT_LTLVISIT_TUNABBREV_HH

#include "ltlast/formula.hh"
#include "ltlvisit/lunabbrev.hh"

namespace spot 
{
  namespace ltl
  {
    class unabbreviate_ltl_visitor : public unabbreviate_logic_visitor
    {
      typedef unabbreviate_logic_visitor super;
    public:
      unabbreviate_ltl_visitor();
      virtual ~unabbreviate_ltl_visitor();

      void visit(const unop* uo);

      formula* recurse(const formula* f);
    };

    formula* unabbreviate_ltl(const formula* f);
  }
}

#endif // SPOT_LTLVISIT_TUNABBREV_HH
