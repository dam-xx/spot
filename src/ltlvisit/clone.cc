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
    clone_visitor::visit(const atomic_prop* ap)
    {
      result_ = new atomic_prop(ap->name());
    }

    void 
    clone_visitor::visit(const constant* c)
    {
      result_ = new constant(c->val());
    }

    void 
    clone_visitor::visit(const unop* uo)
    {
      result_ = new unop(uo->op(), recurse(uo->child()));
    }
    
    void 
    clone_visitor::visit(const binop* bo)
    {
      result_ = new binop(bo->op(), 
			  recurse(bo->first()), recurse(bo->second()));
    }
    
    void 
    clone_visitor::visit(const multop* mo)
    {
      multop* res = new multop(mo->op());
      unsigned mos = mo->size();
      for (unsigned i = 0; i < mos; ++i)
	{
	  res->add(recurse(mo->nth(i)));
	}
      result_ = res;
    }

    formula* 
    clone_visitor::recurse(const formula* f)
    {
      return clone(f);
    }

    formula* 
    clone(const formula* f)
    {
      clone_visitor v;
      f->accept(v);
      return v.result();
    }

  }
}
