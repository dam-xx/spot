#ifndef SPOT_LTLVISIT_LUNABBREV_HH
# define SPOT_LTLVISIT_LUNABBREV_HH

#include "clone.hh"

namespace spot 
{
  namespace ltl
  {
    // This visitor is public, because it's convenient
    // to derive from it and override part of its methods.
    class unabbreviate_logic_visitor : public clone_visitor
    {
      typedef clone_visitor super;
    public:
      unabbreviate_logic_visitor();
      virtual ~unabbreviate_logic_visitor();

      using super::visit;
      void visit(const binop* bo);

      virtual formula* recurse(const formula* f);
    };

    formula* unabbreviate_logic(const formula* f);
  }
}

#endif // SPOT_LTLVISIT_LUNABBREV_HH
