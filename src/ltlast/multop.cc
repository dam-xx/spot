#include <cassert>
#include <utility>
#include "multop.hh"
#include "visitor.hh"

namespace spot
{
  namespace ltl
  {    
    multop::multop(type op, formulae* first, formulae* second)
      : op_(op), children_(2)
    {
      children_[0] = first;
      children_[1] = second;
    }
    
    void
    multop::add(formulae* f)
    {
      children_.push_back(f);
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

    const formulae*
    multop::nth(unsigned n) const
    {
      return children_[n];
    }

    formulae* 
    multop::nth(unsigned n)
    {
      return children_[n];
    }

    multop::type 
    multop::op() const
    {
      return op_;
    }

    bool 
    multop::equals(const formulae* f) const
    {
      // This check is a bit more complicated than other checks
      // because And(a, b, c) is equal to And(c, a, b, a).
      const multop* p1 = dynamic_cast<const multop*>(f);
      if (!p1 || p1->op() != op())
	return false;
      
      const multop* p2 = this;
      unsigned s1 = p1->size();
      unsigned s2 = p2->size();
      if (s1 > s2)
	{
	  std::swap(s1, s2);
	  std::swap(p1, p2);
	}
      
      for (unsigned n1 = 0; n1 < s1; ++n1)
	{
	  unsigned n2;
	  for (n2 = 0; n2 < s2; ++n2)
	    {
	      if (p1->nth(n1)->equals(p2->nth(n2)))
		break;
	    }
	  if (n2 == s2)
	    return false;
	}
      return true;
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
