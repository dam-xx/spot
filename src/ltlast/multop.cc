#include <cassert>
#include <utility>
#include "multop.hh"
#include "visitor.hh"

namespace spot
{
  namespace ltl
  {    
    multop::multop(type op)
      : op_(op)
    {
    }

    multop::multop(type op, formula* first, formula* second)
      : op_(op)
    {
      children_.reserve(2);
      add(first);
      add(second);
    }
    
    void
    multop::add(formula* f)
    {
      // If the formula we add is itself a multop for the same operator,
      // merge its children with ours.
      multop* p = dynamic_cast<multop*>(f);
      if (p && p->op() == op())
	{
	  unsigned ps = p->size();
	  for (unsigned i = 0; i < ps; ++i)
	    children_.push_back(p->nth(i));
	  // that sub-formula is now useless 
	  delete f;
	}
      else
	{
	  children_.push_back(f);
	}
    }

    multop::~multop()
    {
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
      return children_.size();
    }

    const formula*
    multop::nth(unsigned n) const
    {
      return children_[n];
    }

    formula* 
    multop::nth(unsigned n)
    {
      return children_[n];
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

  }
}
