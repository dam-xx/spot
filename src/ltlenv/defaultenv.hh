#ifndef SPOT_LTLENV_DEFAULT_ENVIRONMENT_HH
# define SPOT_LTLENV_DEFAULT_ENVIRONMENT_HH

# include "environment.hh"
# include "ltlast/atomic_prop.hh"

namespace spot
{
  namespace ltl
  {

    class default_environment : public environment
    {
    public:
      virtual formula* require(const std::string& prop_str);
      virtual const std::string& name();

      /* This class is a singleton.  */
      static default_environment& instance();
    protected:
      default_environment();
    };

  }
}

#endif // SPOT_LTLENV_DEFAULT_ENVIRONMENT_HH
