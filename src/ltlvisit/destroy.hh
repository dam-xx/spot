#ifndef SPOT_LTLVISIT_DESTROY_HH
# define SPOT_LTLVISIT_DESTROY_HH

#include "ltlvisit/postfix.hh"

namespace spot
{
  namespace ltl
  {
    /// \brief Destroys a formula
    void destroy(formula *f);
    /// \brief Destroys a formula
    void destroy(const formula *f);
  }
}

#endif // SPOT_LTLVISIT_DESTROY_HH
