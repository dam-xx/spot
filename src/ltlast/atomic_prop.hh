#ifndef SPOT_LTLAST_ATOMIC_PROP_HH
# define SPOT_LTLAST_ATOMIC_PROP_HH

#include <string>
#include "formula.hh"
#include "ltlenv/environment.hh"

namespace spot
{
  namespace ltl
  {
    
    class atomic_prop : public formula
    {
    public:
      atomic_prop(const std::string& name, environment& env);
      virtual ~atomic_prop();

      virtual void accept(visitor& visitor);
      virtual void accept(const_visitor& visitor) const;

      const std::string& name() const;
      environment& env() const;
    private:
      std::string name_;
      environment* env_;
    };

  }
}

#endif // SPOT_LTLAST_ATOMICPROP_HH
