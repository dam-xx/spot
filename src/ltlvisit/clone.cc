#include "ltlast/allnodes.hh"
#include "clone.hh"

namespace spot
{
  namespace ltl
  {
    clone_visitor::clone_visitor()
    {
    }

    clone_visitor::~clone_visitor()
    {
    }

    formula*
    clone_visitor::result() const
    {
      return result_;
    }

    void
    clone_visitor::visit(atomic_prop* ap)
    {
      result_ = ap->ref();
    }

    void
    clone_visitor::visit(constant* c)
    {
      result_ = c->ref();
    }

    void
    clone_visitor::visit(unop* uo)
    {
      result_ = unop::instance(uo->op(), recurse(uo->child()));
    }

    void
    clone_visitor::visit(binop* bo)
    {
      result_ = binop::instance(bo->op(),
				recurse(bo->first()), recurse(bo->second()));
    }

    void
    clone_visitor::visit(multop* mo)
    {
      multop::vec* res = new multop::vec;
      unsigned mos = mo->size();
      for (unsigned i = 0; i < mos; ++i)
	res->push_back(recurse(mo->nth(i)));
      result_ = multop::instance(mo->op(), res);
    }

    formula*
    clone_visitor::recurse(formula* f)
    {
      return clone(f);
    }

    formula*
    clone(const formula* f)
    {
      clone_visitor v;
      const_cast<formula*>(f)->accept(v);
      return v.result();
    }
  }
}
