#include "atomic_prop.hh"
#include "visitor.hh"
#include <cassert>

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
      // Get this instance out of the instance map.
      pair p(name(), &env());
      map::iterator i = instances.find(p);
      assert (i != instances.end());
      instances.erase(i);
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

    atomic_prop::map atomic_prop::instances;

    atomic_prop*
    atomic_prop::instance(const std::string& name, environment& env)
    {
      pair p(name, &env);
      map::iterator i = instances.find(p);
      if (i != instances.end())
	{
	  return static_cast<atomic_prop*>(i->second->ref());
	}
      atomic_prop* ap = new atomic_prop(name, env);
      instances[p] = ap;
      return static_cast<atomic_prop*>(ap->ref());
    }

    unsigned
    atomic_prop::instance_count()
    {
      return instances.size();
    }

  }
}
