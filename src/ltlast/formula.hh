#ifndef SPOT_LTLAST_FORMULAE_HH
# define SPOT_LTLAST_FORMULAE_HH

#include "predecl.hh"

namespace spot 
{
  namespace ltl 
  {

    /// \brief An LTL formula.
    /// 
    /// The only way you can work with a formula is to 
    /// build a spot::ltl::visitor or spot::ltl::const_visitor.
    class formula 
    {
    public:
      virtual void accept(visitor& v) = 0;
      virtual void accept(const_visitor& v) const = 0;
    };

  }
}



#endif // SPOT_LTLAST_FORMULAE_HH
