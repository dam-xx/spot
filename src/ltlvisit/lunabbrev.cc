#include "ltlast/allnodes.hh"
#include "ltlvisit/clone.hh"
#include "lunabbrev.hh"
#include <cassert>

namespace spot
{
  namespace ltl
  {
    unabbreviate_logic_visitor::unabbreviate_logic_visitor()
    {
    }

    unabbreviate_logic_visitor::~unabbreviate_logic_visitor()
    {
    }

    void
    unabbreviate_logic_visitor::visit(binop* bo)
    {
      formula* f1 = recurse(bo->first());
      formula* f2 = recurse(bo->second());
      switch (bo->op())
	{
	  /* f1 ^ f2  ==  (f1 & !f2) | (f2 & !f1) */
	case binop::Xor:
	  result_ = multop::instance(multop::Or,
				     multop::instance(multop::And, clone(f1),
						      unop::instance(unop::Not,
								     f2)),
				     multop::instance(multop::And, clone(f2),
						      unop::instance(unop::Not,
								     f1)));
	  return;
	  /* f1 => f2  ==  !f1 | f2 */
	case binop::Implies:
	  result_ = multop::instance(multop::Or,
				     unop::instance(unop::Not, f1), f2);
	  return;
	  /* f1 <=> f2  ==  (f1 & f2) | (!f1 & !f2) */
	case binop::Equiv:
	  result_ = multop::instance(multop::Or,
				     multop::instance(multop::And,
						      clone(f1), clone(f2)),
				     multop::instance(multop::And,
						      unop::instance(unop::Not,
								     f1),
						      unop::instance(unop::Not,
								     f2)));
	  return;
	  /* f1 U f2 == f1 U f2 */
	  /* f1 R f2 == f1 R f2 */
	case binop::U:
	case binop::R:
	  result_ = binop::instance(bo->op(), f1, f2);
	  return;
	}
      /* Unreachable code. */
      assert(0);
    }

    formula*
    unabbreviate_logic_visitor::recurse(formula* f)
    {
      return unabbreviate_logic(f);
    }

    formula*
    unabbreviate_logic(formula* f)
    {
      unabbreviate_logic_visitor v;
      f->accept(v);
      return v.result();
    }

  }
}
