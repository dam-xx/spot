#ifndef SPOT_LTLENV_ENVIRONMENT_HH
# define SPOT_LTLENV_ENVIRONMENT_HH

# include "ltlast/formula.hh"
# include <string>

namespace spot
{
  namespace ltl
  {

    class environment
    {
    public:
      // Check whether the environment contains the atomic proposition
      // described by prop_str.
      // Note this is NOT a const method.  Some environment will
      // "create" the atomic proposition when asked.
      // We return a formula instead of an atomic_prop, because this
      // will allow nifty tricks (e.g., we could name formulae in an
      // environment, and let the parser build a larger tree from
      // these).
      virtual formula* require(const std::string& prop_str) = 0; 

      virtual const std::string& name() = 0;
      // FIXME: More functions will be needed later, but
      // it's enough for now.
    };

  }
}

#endif // SPOT_LTLENV_ENVIRONMENT_HH
