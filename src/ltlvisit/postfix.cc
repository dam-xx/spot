#include "ltlvisit/postfix.hh"
#include "ltlast/allnodes.hh"

namespace spot
{
  namespace ltl
  {
    postfix_visitor::postfix_visitor()
    {
    }

    postfix_visitor::~postfix_visitor()
    {
    }

    void
    postfix_visitor::visit(atomic_prop* ap)
    {
      doit(ap);
    }

    void
    postfix_visitor::visit(unop* uo)
    {
      uo->child()->accept(*this);
      doit(uo);
    }

    void
    postfix_visitor::visit(binop* bo)
    {
      bo->first()->accept(*this);
      bo->second()->accept(*this);
      doit(bo);
    }

    void
    postfix_visitor::visit(multop* mo)
    {
      unsigned s = mo->size();
      for (unsigned i = 0; i < s; ++i)
	mo->nth(i)->accept(*this);
      doit(mo);
    }

    void
    postfix_visitor::visit(constant* c)
    {
      doit(c);
    }

    void
    postfix_visitor::doit(atomic_prop* ap)
    {
      doit_default(ap);
    }

    void
    postfix_visitor::doit(unop* uo)
    {
      doit_default(uo);
    }

    void
    postfix_visitor::doit(binop* bo)
    {
      doit_default(bo);
    }

    void
    postfix_visitor::doit(multop* mo)
    {
      doit_default(mo);
    }

    void
    postfix_visitor::doit(constant* c)
    {
      doit_default(c);
    }

    void
    postfix_visitor::doit_default(formula* f)
    {
      (void)f;
      // Dummy implementation that does nothing.
    }
  }
}
