#include <cassert>
#include <utility>
#include "multop.hh"
#include "visitor.hh"

namespace spot
{
  namespace ltl
  {
    multop::multop(type op, vec* v)
      : op_(op), children_(v)
    {
    }

    multop::~multop()
    {
      // Get this instance out of the instance map.
      pair p(op(), children_);
      map::iterator i = instances.find(p);
      assert (i != instances.end());
      instances.erase(i);

      delete children_;
    }

    void
    multop::accept(visitor& v)
    {
      v.visit(this);
    }

    void
    multop::accept(const_visitor& v) const
    {
      v.visit(this);
    }

    unsigned
    multop::size() const
    {
      return children_->size();
    }

    const formula*
    multop::nth(unsigned n) const
    {
      return (*children_)[n];
    }

    formula*
    multop::nth(unsigned n)
    {
      return (*children_)[n];
    }

    multop::type
    multop::op() const
    {
      return op_;
    }

    const char*
    multop::op_name() const
    {
      switch (op_)
	{
	case And:
	  return "And";
	case Or:
	  return "Or";
	}
      // Unreachable code.
      assert(0);
      return 0;
    }

    multop::map multop::instances;

    multop*
    multop::instance(type op, vec* v)
    {
      pair p(op, v);
      map::iterator i = instances.find(p);
      if (i != instances.end())
	{
	  delete v;
	  return static_cast<multop*>(i->second->ref());
	}
      multop* ap = new multop(op, v);
      instances[p] = ap;
      return static_cast<multop*>(ap->ref());

    }

    multop*
    multop::instance(type op)
    {
      return instance(op, new vec);
    }

    multop*
    multop::instance(type op, formula* first, formula* second)
    {
      vec* v = new vec;
      multop::add(op, v, first);
      multop::add(op, v, second);
      return instance(op, v);
    }

    multop::vec*
    multop::add(type op, vec* v, formula* f)
    {
      // If the formula we add is itself a multop for the same operator,
      // merge its children.
      multop* p = dynamic_cast<multop*>(f);
      if (p && p->op() == op)
	{
	  unsigned ps = p->size();
	  for (unsigned i = 0; i < ps; ++i)
	    v->push_back(p->nth(i));
	  // that sub-formula is now useless
	  formula::unref(f);
	}
      else
	{
	  v->push_back(f);
	}
      return v;
    }

    void
    multop::add(multop** m, formula* f)
    {
      vec* v = new vec(*(*m)->children_);
      type op = (*m)->op();
      multop::add(op, v, f);
      formula::unref(*m);
      *m = instance(op, v);
    }

    unsigned
    multop::instance_count()
    {
      return instances.size();
    }
  }
}
