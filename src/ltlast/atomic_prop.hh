#ifndef SPOT_LTLAST_ATOMIC_PROP_HH
# define SPOT_LTLAST_ATOMIC_PROP_HH

#include <string>
#include <map>
#include "refformula.hh"
#include "ltlenv/environment.hh"

namespace spot
{
  namespace ltl
  {

    /// Atomic propositions.
    class atomic_prop : public ref_formula
    {
    public:
      /// Build an atomic proposition with name \a name in
      /// environment \a env.
      static atomic_prop* instance(const std::string& name, environment& env);

      virtual void accept(visitor& visitor);
      virtual void accept(const_visitor& visitor) const;

      /// Get the name of the atomic proposition.
      const std::string& name() const;
      /// Get the environment of the atomic proposition.
      environment& env() const;
    protected:
      atomic_prop(const std::string& name, environment& env);
      virtual ~atomic_prop();

      typedef std::pair<std::string, environment*> pair;
      typedef std::map<pair, atomic_prop*> map;
      static map instances;

    private:
      std::string name_;
      environment* env_;
    };

  }
}

#endif // SPOT_LTLAST_ATOMICPROP_HH
