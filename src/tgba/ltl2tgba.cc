#include "ltlast/visitor.hh"
#include "ltlast/allnodes.hh"
#include "tgbabddconcretefactory.hh"

#include "ltl2tgba.hh"

namespace spot
{
  using namespace ltl;

  /// \brief Recursively translate a formula into a BDD.
  ///
  /// The algorithm used here is adapted from Jean-Michel Couvreur's
  /// Probataf tool.
  class ltl_trad_visitor: public const_visitor
  {
  public:
    ltl_trad_visitor(tgba_bdd_concrete_factory& fact)
      : fact_(fact)
    {
    }

    virtual
    ~ltl_trad_visitor()
    {
    }

    bdd
    result()
    {
      return res_;
    }

    void
    visit(const atomic_prop* node)
    {
      res_ = fact_.ithvar(fact_.create_atomic_prop(node));
    }

    void
    visit(const constant* node)
    {
      switch (node->val())
	{
	case constant::True:
	  res_ = bddtrue;
	  return;
	case constant::False:
	  res_ = bddfalse;
	  return;
	}
      /* Unreachable code.  */
      assert(0);
    }

    void
    visit(const unop* node)
    {
      switch (node->op())
	{
	case unop::F:
	case unop::G:
	  // FIXME: We can normalize on the fly, here.
	  assert(!"unexpected operator, normalize first");
	case unop::Not:
	  res_ = bdd_not(recurse(node->child()));
	  return;
	case unop::X:
	  // FIXME: Can be smarter on X(a U b) and X(a R b).
	  int v = fact_.create_state(node->child());
	  bdd now = fact_.ithvar(v);
	  bdd next = fact_.ithvar(v + 1);
	  fact_.add_relation(bdd_apply(now, recurse(node->child()),
				       bddop_biimp));
	  res_ = next;
	  return;
	}
      /* Unreachable code.  */
      assert(0);
    }

    void
    visit(const binop* node)
    {
      bdd f1 = recurse(node->first());
      bdd f2 = recurse(node->second());

      switch (node->op())
	{
	case binop::Xor:
	  res_ = bdd_apply(f1, f2, bddop_xor);
	  return;
	case binop::Implies:
	  res_ = bdd_apply(f1, f2, bddop_imp);
	  return;
	case binop::Equiv:
	  res_ = bdd_apply(f1, f2, bddop_biimp);
	  return;
	case binop::U:
	  {
	    int v = fact_.create_state(node);
	    bdd now = fact_.ithvar(v);
	    bdd next = fact_.ithvar(v + 1);

	    bdd promise_f2 =
	      fact_.ithvar(fact_.create_promise(node->second()));
	    /*
	       f1 U f2 <=> f2 | (f1 & X(f1 U f2))
	       In other words:
		   now <=> f2 | (f1 & next)

		The rightmost conjunction, f1 & next, doesn't actually
		encodes the fact that f2 should be fulfilled at some
		point.  We use the `promise_f2' variable for this purpose.
	    */
	    fact_.add_relation(bdd_apply(now,
					 f2 | (promise_f2 & f1 & next),
					 bddop_biimp));
	    res_ = now;
	    return;
	  }
	case binop::R:
	  {
	    int v = fact_.create_state(node);
	    bdd now = fact_.ithvar(v);
	    bdd next = fact_.ithvar(v + 1);
	    /*
	       f1 R f2 <=> f2 & (f1 | X(f1 U f2))
	       In other words:
		   now <=> f2 & (f1 | next)
	    */
	    fact_.add_relation(bdd_apply(now, f2 & (f1 | next), bddop_biimp));
	    res_ = now;
	    return;
	  }
	}
      /* Unreachable code.  */
      assert(0);
    }

    void
    visit(const multop* node)
    {
      int op = -1;
      switch (node->op())
	{
	case multop::And:
	  op = bddop_and;
	  res_ = bddtrue;
	  break;
	case multop::Or:
	  op = bddop_or;
	  res_ = bddfalse;
	  break;
	}
      assert(op != -1);
      unsigned s = node->size();
      for (unsigned n = 0; n < s; ++n)
	{
	  res_ = bdd_apply(res_, recurse(node->nth(n)), op);
	}
    }

    bdd
    recurse(const formula* f)
    {
      ltl_trad_visitor v(*this);
      f->accept(v);
      return v.result();
    }

  private:
    bdd res_;
    tgba_bdd_concrete_factory& fact_;
  };

  tgba_bdd_concrete
  ltl_to_tgba(const ltl::formula* f)
  {
    tgba_bdd_concrete_factory fact;
    ltl_trad_visitor v(fact);
    f->accept(v);
    tgba_bdd_concrete g(fact);
    g.set_init_state(v.result());
    return g;
  }
}
