#ifndef SPOT_LTLAST_ATOMIC_PROP_HH
# define SPOT_LTLAST_ATOMIC_PROP_HH

#include <string>
#include "formulae.hh"

namespace spot
{
  namespace ltl
  {
    
    class atomic_prop : public formulae
    {
    public:
      atomic_prop(std::string name);
      virtual ~atomic_prop();

      virtual void accept(visitor& visitor);
      virtual void accept(const_visitor& visitor) const;

      const std::string& name() const;
    private:
      std::string name_;
    };

  }
}

#endif // SPOT_LTLAST_ATOMICPROP_HH
