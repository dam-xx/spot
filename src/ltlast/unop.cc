#include "unop.hh"
#include "visitor.hh"
#include <cassert>

namespace spot
{
  namespace ltl
  {    
    unop::unop(type op, formulae* child)
      : op_(op), child_(child)
    {
    }

    unop::~unop()
    {
    }

    void
    unop::accept(visitor& v)
    {
      v.visit(this);
    }

    void
    unop::accept(const_visitor& v) const
    {
      v.visit(this);
    }

    const formulae*
    unop::child() const
    {
      return child_;
    }

    formulae*
    unop::child()
    {
      return child_;
    }

    bool 
    unop::equals(const formulae* f) const
    {
      const unop* p = dynamic_cast<const unop*>(f);
      return p && p->op() == op() && child()->equals(p->child());
    }

    unop::type 
    unop::op() const
    {
      return op_;
    }

    const char* 
    unop::op_name() const
    {
      switch (op_)
	{
	case Not:
	  return "Not";
	case X:
	  return "X";
	case F:
	  return "F";
	case G:
	  return "G";
	}
      // Unreachable code.
      assert(0);
      return 0;
    }

  }
}
