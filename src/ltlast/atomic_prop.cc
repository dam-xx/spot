#include "atomic_prop.hh"
#include "visitor.hh"

namespace spot
{
  namespace ltl
  {
 
    atomic_prop::atomic_prop(std::string name)
      : name_(name)
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

    bool 
    atomic_prop::equals(const formulae* f) const
    {
      const atomic_prop* p = dynamic_cast<const atomic_prop*>(f);
      return p && p->name() == name();
    }

    const std::string&
    atomic_prop::name() const
    {
      return name_;
    }
  }
}
