#include "defaultenv.hh"

namespace spot
{
  namespace ltl
  {

    formula*
    default_environment::require(const std::string& s)
    {
      return new atomic_prop(s, *this);
    }

    const std::string&
    default_environment::name()
    {
      static std::string name("default environment");
      return name;
    }

    default_environment::default_environment()
    {
    }

    default_environment& 
    default_environment::instance()
    {
      static default_environment* singleton = new default_environment();
      return *singleton;
    }

  }
}
