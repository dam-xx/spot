#include "ltlast/allnodes.hh"
#include "tunabbrev.hh"

namespace spot 
{
  namespace ltl
  {
    unabbreviate_ltl_visitor::unabbreviate_ltl_visitor()
    {
    }

    unabbreviate_ltl_visitor::~unabbreviate_ltl_visitor()
    {
    }

    void 
    unabbreviate_ltl_visitor::visit(const unop* uo)
    {
      switch (uo->op())
	{
	case unop::X:
	case unop::Not:
	  this->super::visit(uo);
	  return;
	case unop::F:
	  result_ = new binop(binop::U, 
			      new constant(constant::True),
			      recurse(uo->child()));
	  return;
	case unop::G:
	  result_ = new binop(binop::R, 
			      new constant(constant::False),
			      recurse(uo->child()));
	  return;
	}
    }
    
    formula* 
    unabbreviate_ltl_visitor::recurse(const formula* f)
    {
      return unabbreviate_ltl(f);
    }

    formula* 
    unabbreviate_ltl(const formula* f)
    {
      unabbreviate_ltl_visitor v;
      f->accept(v);
      return v.result();
    }

  }
}
