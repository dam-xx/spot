#include "constant.hh"
#include "visitor.hh"
#include <cassert>

namespace spot
{
  namespace ltl
  {    
    constant::constant(type val)
      : val_(val)
    {
    }

    constant::~constant()
    {
    }

    void
    constant::accept(visitor& v)
    {
      v.visit(this);
    }

    void
    constant::accept(const_visitor& v) const
    {
      v.visit(this);
    }

    constant::type 
    constant::val() const
    {
      return val_;
    }

    const char* 
    constant::val_name() const
    {
      switch (val_)
	{
	case True:
	  return "1";
	case False:
	  return "0";
	}
      // Unreachable code.
      assert(0);
      return 0;
    }

  }
}
