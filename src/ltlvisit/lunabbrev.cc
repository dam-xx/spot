#include "ltlast/allnodes.hh"
#include "lunabbrev.hh"

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

    formula*
    unabbreviate_logic_visitor::result() const
    {
      return result_;
    }
    
    void 
    unabbreviate_logic_visitor::visit(const atomic_prop* ap)
    {
      result_ = new atomic_prop(ap->name());
    }

    void 
    unabbreviate_logic_visitor::visit(const constant* c)
    {
      result_ = new constant(c->val());
    }

    void 
    unabbreviate_logic_visitor::visit(const unop* uo)
    {
      result_ = new unop(uo->op(), recurse(uo->child()));
    }
    
    void 
    unabbreviate_logic_visitor::visit(const binop* bo)
    {
      formula* f1 = recurse(bo->first());
      formula* f2 = recurse(bo->second());
      switch (bo->op())
	{
	  /* f1 ^ f2  ==  (f1 & !f2) | (f2 & !f1) */
	case binop::Xor:
	  result_ = new multop(multop::Or,
			       new multop(multop::And, f1,
					  new unop(unop::Not, f2)),
			       new multop(multop::And, f2,
					  new unop(unop::Not, f1)));
	  return;
	  /* f1 => f2  ==  !f1 | f2 */
	case binop::Implies:
	  result_ = new multop(multop::Or, new unop(unop::Not, f1), f2);
	  return;
	  /* f1 <=> f2  ==  (f1 & f2) | (!f1 & !f2) */
	case binop::Equiv:
	  result_ = new multop(multop::Or,
			       new multop(multop::And, f1, f2),
			       new multop(multop::And, 
					  new unop(unop::Not, f1),
					  new unop(unop::Not, f2)));
	  return;
	  /* f1 U f2 == f1 U f2 */
	  /* f1 R f2 == f1 R f2 */
	case binop::U:
	case binop::R:
	  result_ = new binop(bo->op(), f1, f2);
	  return;
	}
      /* Unreachable code. */
      assert(0);
    }
    
    void 
    unabbreviate_logic_visitor::visit(const multop* mo)
    {
      multop* res = new multop(mo->op());
      unsigned mos = mo->size();
      for (unsigned i = 0; i < mos; ++i)
	{
	  res->add(recurse(mo->nth(i)));
	}
      result_ = res;
    }

    formula* unabbreviate_logic_visitor::recurse(const formula* f)
    {
      return unabbreviate_logic(f);
    }

    formula* 
    unabbreviate_logic(const formula* f)
    {
      unabbreviate_logic_visitor v;
      f->accept(v);
      return v.result();
    }

  }
}
