#include <cassert>
#include "binop.hh"
#include "visitor.hh"

namespace spot
{
  namespace ltl
  {    
    binop::binop(type op, formula* first, formula* second)
      : op_(op), first_(first), second_(second)
    {
    }

    binop::~binop()
    {
    }

    void
    binop::accept(visitor& v)
    {
      v.visit(this);
    }

    void
    binop::accept(const_visitor& v) const
    {
      v.visit(this);
    }

    const formula*
    binop::first() const
    {
      return first_;
    }

    formula*
    binop::first()
    {
      return first_;
    }

    const formula*
    binop::second() const
    {
      return second_;
    }

    formula* 
    binop::second()
    {
      return second_;
    }

    binop::type 
    binop::op() const
    {
      return op_;
    }

    const char* 
    binop::op_name() const
    {
      switch (op_)
	{
	case Xor:
	  return "Xor";
	case Implies:
	  return "Implies";
	case Equiv:
	  return "Equiv";
	case U:
	  return "U";
	case R:
	  return "R";
	}
      // Unreachable code.
      assert(0);
      return 0;
    }

  }
}
