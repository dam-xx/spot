#include "ltlast/visitor.hh"
#include "ltlast/allnodes.hh"
#include "ltlvisit/lunabbrev.hh"
#include "ltlvisit/nenoform.hh"
#include "ltlvisit/destroy.hh"
#include "tgba/tgbabddconcretefactory.hh"
#include <cassert>

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
    ltl_trad_visitor(tgba_bdd_concrete_factory& fact, bool root = false)
      : fact_(fact), root_(root)
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
	  {
	    /*
		   Fx  <=> x | XFx
	       In other words:
		   now <=> x | next
	    */
	    int v = fact_.create_state(node);
	    bdd now = fact_.ithvar(v);
	    bdd next = fact_.ithvar(v + 1);
	    bdd x = recurse(node->child());
	    fact_.constrain_relation(bdd_apply(now, x | next, bddop_biimp));
	    /*
	      `x | next', doesn't actually encode the fact that x
	      should be fulfilled eventually.  We ensure this by
	      creating a new generalized Büchi accepting set, Acc[x],
	      and leave out of this set any transition going off NOW
	      without checking X.  Such accepting conditions are
	      checked for during the emptiness check.
	    */
	    fact_.declare_accepting_condition(x | !now, node->child());
	    res_ = now;
	    return;
	  }
	case unop::G:
	  {
	    bdd child = recurse(node->child());
	    // If G occurs at the top of the formula we don't
	    // need Now/Next variables.  We just constrain
	    // the relation so that the child always happens.
	    // This saves 2 BDD variables.
	    if (root_)
	      {
		fact_.constrain_relation(child);
		res_ = child;
		return;
	      }
	    // Gx  <=>  x && XGx
	    int v = fact_.create_state(node);
	    bdd now = fact_.ithvar(v);
	    bdd next = fact_.ithvar(v + 1);
	    fact_.constrain_relation(bdd_apply(now, child & next,
					       bddop_biimp));
	    res_ = now;
	    return;
	  }
	case unop::Not:
	  {
	    res_ = bdd_not(recurse(node->child()));
	    return;
	  }
	case unop::X:
	  {
	    int v = fact_.create_state(node->child());
	    bdd now = fact_.ithvar(v);
	    bdd next = fact_.ithvar(v + 1);
	    fact_.constrain_relation(bdd_apply(now, recurse(node->child()),
					       bddop_biimp));
	    res_ = next;
	    return;
	  }
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
	    /*
	       f1 U f2 <=> f2 | (f1 & X(f1 U f2))
	       In other words:
		   now <=> f2 | (f1 & next)
	    */
	    int v = fact_.create_state(node);
	    bdd now = fact_.ithvar(v);
	    bdd next = fact_.ithvar(v + 1);
	    fact_.constrain_relation(bdd_apply(now, f2 | (f1 & next),
					       bddop_biimp));
	    /*
	      The rightmost conjunction, f1 & next, doesn't actually
	      encode the fact that f2 should be fulfilled eventually.
	      We declare an accepting condition for this purpose (see
	      the comment in the unop::F case).
	    */
	    fact_.declare_accepting_condition(f2 | !now, node->second());
	    res_ = now;
	    return;
	  }
	case binop::R:
	  {
	    /*
	       f1 R f2 <=> f2 & (f1 | X(f1 U f2))
	       In other words:
		   now <=> f2 & (f1 | next)
	    */
	    int v = fact_.create_state(node);
	    bdd now = fact_.ithvar(v);
	    bdd next = fact_.ithvar(v + 1);
	    fact_.constrain_relation(bdd_apply(now, f2 & (f1 | next),
					       bddop_biimp));
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
      ltl_trad_visitor v(fact_);
      f->accept(v);
      return v.result();
    }

  private:
    bdd res_;
    tgba_bdd_concrete_factory& fact_;
    bool root_;
  };

  tgba_bdd_concrete
  ltl_to_tgba(const ltl::formula* f)
  {
    // Normalize the formula.  We want all the negation on
    // the atomic proposition.  We also suppress logic
    // abbreviation such as <=>, =>, or XOR, since they
    // would involve negations at the BDD level.
    const ltl::formula* f1 = ltl::unabbreviate_logic(f);
    const ltl::formula* f2 = ltl::negative_normal_form(f1);
    ltl::destroy(f1);

    // Traverse the formula and draft the automaton in a factory.
    tgba_bdd_concrete_factory fact;
    ltl_trad_visitor v(fact, true);
    f2->accept(v);
    ltl::destroy(f2);
    fact.finish();

    // Finally setup the resulting automaton.
    tgba_bdd_concrete g(fact, v.result());
    return g;
  }
}
