#ifndef SPOT_LTLENV_DEFAULT_ENVIRONMENT_HH
# define SPOT_LTLENV_DEFAULT_ENVIRONMENT_HH

# include "environment.hh"

namespace spot
{
  namespace ltl
  {

    /// \brief A laxist environment.
    ///
    /// This environment recognizes all atomic propositions.
    ///
    /// This is a singleton.  Use default_environment::instance()
    /// to obtain the instance.
    class default_environment : public environment
    {
    public:
      virtual ~default_environment();
      virtual formula* require(const std::string& prop_str);
      virtual const std::string& name();

      /// Get the sole instance of spot::ltl::default_environment.
      static default_environment& instance();
    protected:
      default_environment();
    };

  }
}

#endif // SPOT_LTLENV_DEFAULT_ENVIRONMENT_HH
