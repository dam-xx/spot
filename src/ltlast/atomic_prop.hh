#ifndef SPOT_LTLAST_ATOMIC_PROP_HH
# define SPOT_LTLAST_ATOMIC_PROP_HH

#include <string>
#include "formula.hh"
#include "ltlenv/environment.hh"

namespace spot
{
  namespace ltl
  {

    /// Atomic propositions.
    class atomic_prop : public formula
    {
    public:
      /// Build an atomic proposition with name \a name in 
      /// environment \a env.
      atomic_prop(const std::string& name, environment& env);
      virtual ~atomic_prop();

      virtual void accept(visitor& visitor);
      virtual void accept(const_visitor& visitor) const;

      /// Get the name of the atomic proposition.
      const std::string& name() const;
      /// Get the environment of the atomic proposition.
      environment& env() const;
    private:
      std::string name_;
      environment* env_;
    };

  }
}

#endif // SPOT_LTLAST_ATOMICPROP_HH
