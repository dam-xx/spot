#ifndef SPOT_LTLAST_FORMULAE_HH
# define SPOT_LTLAST_FORMULAE_HH

#include "predecl.hh"

namespace spot 
{
  namespace ltl 
  {

    class formula 
    {
    public:
      virtual void accept(visitor& v) = 0;
      virtual void accept(const_visitor& v) const = 0;
    };

  }
}



#endif // SPOT_LTLAST_FORMULAE_HH
