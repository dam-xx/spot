#include "nenoform.hh"
#include "ltlast/allnodes.hh"

namespace spot
{
  namespace ltl
  {

    class negative_normal_form_visitor : public const_visitor
    {
    public:
      negative_normal_form_visitor(bool negated)
	: negated_(negated)
      {
      }

      virtual 
      ~negative_normal_form_visitor()
      {
      }

      formula* result() const
      {
	return result_;
      }
      
      void 
      visit(const atomic_prop* ap)
      {
	formula* f = new atomic_prop(ap->name(), ap->env());
	if (negated_)
	  result_ = new unop(unop::Not, f);
	else
	  result_ = f;
      }

      void 
      visit(const constant* c)
      {
	if (! negated_)
	  {
	    result_ = new constant(c->val());
	    return;
	  }

	switch (c->val())
	  {
	  case constant::True:
	    result_ = new constant(constant::False);
	    return;
	  case constant::False:
	    result_ = new constant(constant::True);
	    return;
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void 
      visit(const unop* uo)
      {
	const formula* f = uo->child();
	switch (uo->op())
	  {
	  case unop::Not:
	    result_ = recurse_(f, negated_ ^ true);
	    return;
	  case unop::X:
	    /* !Xa == X!a */
	    result_ = new unop(unop::X, recurse(f));
	    return;
	  case unop::F:
	    /* !Fa == G!a */
	    result_ = new unop(negated_ ? unop::G : unop::F, recurse(f));
	    return;
	  case unop::G:
	    /* !Ga == F!a */
	    result_ = new unop(negated_ ? unop::F : unop::G, recurse(f));
	    return;
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void 
      visit(const binop* bo)
      {
	const formula* f1 = bo->first();
	const formula* f2 = bo->second();
	switch (bo->op())
	  {
	  case binop::Xor:
	    /* !(a ^ b) == a <=> b */
	    result_ = new binop(negated_ ? binop::Equiv : binop::Xor,
				recurse_(f1, false), recurse_(f2, false));
	    return;
	  case binop::Equiv:
	    /* !(a <=> b) == a ^ b */
	    result_ = new binop(negated_ ? binop::Xor : binop::Equiv,
				recurse_(f1, false), recurse_(f2, false));
	    return;
	  case binop::Implies:
	    if (negated_)
	      /* !(a => b) == a & !b */
	      result_ = new multop(multop::And,
				   recurse_(f1, false), recurse_(f2, true));
	    else
	      result_ = new binop(binop::Implies, recurse(f1), recurse(f2));
	    return;
	  case binop::U:
	    /* !(a U b) == !a R !b */
	    result_ = new binop(negated_ ? binop::R : binop::U,
				recurse(f1), recurse(f2));
	    return;
	  case binop::R:
	    /* !(a R b) == !a U !b */
	    result_ = new binop(negated_ ? binop::U : binop::R,
				recurse(f1), recurse(f2));
	    return;
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void 
      visit(const multop* mo)
      {
	/* !(a & b & c) == !a | !b | !c  */
	/* !(a | b | c) == !a & !b & !c  */
	multop::type op = mo->op();
	if (negated_)
	  switch (op)
	    {
	    case multop::And:
	      op = multop::Or;
	      break;
	    case multop::Or:
	      op = multop::And;
	      break;
	    }
	multop* res = new multop(op);
	unsigned mos = mo->size();
	for (unsigned i = 0; i < mos; ++i)
	  res->add(recurse(mo->nth(i)));
	result_ = res;
      }
	
      formula* 
      recurse_(const formula* f, bool negated)
      {
	return negative_normal_form(f, negated);
      }

      formula* 
      recurse(const formula* f)
      {
	return recurse_(f, negated_);
      }
      
    protected:
      formula* result_;
      bool negated_;
    };

    formula* 
    negative_normal_form(const formula* f, bool negated)
    {
      negative_normal_form_visitor v(negated);
      f->accept(v);
      return v.result();
    }
  }
}
