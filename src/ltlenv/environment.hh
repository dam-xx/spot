#ifndef SPOT_LTLENV_ENVIRONMENT_HH
# define SPOT_LTLENV_ENVIRONMENT_HH

# include "ltlast/formula.hh"
# include <string>

namespace spot
{
  namespace ltl
  {

    /// An environment that describe atomic propositions.
    class environment
    {
    public:
      /// \brief Obtain the formula associated to \a prop_str
      ///
      /// Usually \a prop_str, is the name of an atomic proposition,
      /// a spot::ltl::require simply returns the associated
      /// spot::ltl::atomic_prop.
      ///
      /// Note this is not a \c const method.  Some environment will
      /// "create" the atomic proposition when asked.
      ///
      /// We return a spot::ltl::formula instead of an
      /// spot::ltl::atomic_prop, because this
      /// will allow nifty tricks (e.g., we could name formulae in an
      /// environment, and let the parser build a larger tree from
      /// these).
      ///
      /// \return 0 iff \a prop_str is not part of the environment,
      ///   or the associated spot::ltl::formula otherwise.
      virtual formula* require(const std::string& prop_str) = 0;

      /// Get the name of the environment.
      virtual const std::string& name() = 0;

      // FIXME: More functions will be needed later, but
      // it's enough for now.
    };

  }
}

#endif // SPOT_LTLENV_ENVIRONMENT_HH
