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
    unabbreviate_ltl_visitor::visit(unop* uo)
    {
      switch (uo->op())
	{
	case unop::X:
	case unop::Not:
	  this->super::visit(uo);
	  return;
	case unop::F:
	  result_ = binop::instance(binop::U,
				    constant::true_instance(),
				    recurse(uo->child()));
	  return;
	case unop::G:
	  result_ = binop::instance(binop::R,
				    constant::false_instance(),
				    recurse(uo->child()));
	  return;
	}
    }

    formula*
    unabbreviate_ltl_visitor::recurse(formula* f)
    {
      return unabbreviate_ltl(f);
    }

    formula*
    unabbreviate_ltl(formula* f)
    {
      unabbreviate_ltl_visitor v;
      f->accept(v);
      return v.result();
    }

  }
}
