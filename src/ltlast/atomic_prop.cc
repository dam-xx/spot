#include "atomic_prop.hh"
#include "visitor.hh"

namespace spot
{
  namespace ltl
  {
 
    atomic_prop::atomic_prop(const std::string& name, environment& env)
      : name_(name), env_(&env)
    {
    }

    atomic_prop::~atomic_prop()
    {
    }

    void 
    atomic_prop::accept(visitor& v)
    {
      v.visit(this);
    }
    
    void
    atomic_prop::accept(const_visitor& v) const
    {
      v.visit(this);
    }   

    const std::string&
    atomic_prop::name() const
    {
      return name_;
    }
    
    environment& 
    atomic_prop::env() const
    {
      return *env_;
    }

  }
}
