#ifndef SPOT_LTLAST_REFFORMULAE_HH
# define SPOT_LTLAST_REFFORMULAE_HH

#include "formula.hh"

namespace spot
{
  namespace ltl
  {

    /// \brief A reference-counted LTL formula.
    class ref_formula : public formula
    {
    protected:
      ref_formula();
      void ref_();
      bool unref_();
    private:
      unsigned ref_count_;
    };

  }
}

#endif // SPOT_LTLAST_REFFORMULAE_HH
