#include <vector>
#include "equals.hh"
#include "ltlast/allnodes.hh"

namespace spot
{
  namespace ltl
  {
    equals_visitor::equals_visitor(const formula* f)
      : f_(f), result_(false)
    {
    }

    equals_visitor::~equals_visitor()
    {
    }

    bool
    equals_visitor::result() const
    {
      return result_;
    }

    void
    equals_visitor::visit(const atomic_prop* ap)
    {
      result_ = f_ == ap;
    }

    void
    equals_visitor::visit(const constant* c)
    {
      result_ = f_ == c;
    }

    void
    equals_visitor::visit(const unop* uo)
    {
      const unop* p = dynamic_cast<const unop*>(f_);
      if (!p || p->op() != uo->op())
	return;
      f_ = p->child();
      uo->child()->accept(*this);
    }

    void
    equals_visitor::visit(const binop* bo)
    {
      const binop* p = dynamic_cast<const binop*>(f_);
      if (!p || p->op() != bo->op())
	return;

      // The current visitor will descend the left branch.
      // Build a second visitor for the right branch.
      equals_visitor v2(p->second());
      f_ = p->first();

      bo->first()->accept(*this);
      if (result_ == false)
	return;

      bo->second()->accept(v2);
      result_ = v2.result();
    }

    void
    equals_visitor::visit(const multop* m)
    {
      const multop* p = dynamic_cast<const multop*>(f_);
      if (!p || p->op() != m->op())
	return;

      // This check is a bit more complicated than other checks
      // because And(a, b, c) is equal to And(c, a, b, a).

      unsigned m_size = m->size();
      unsigned p_size = p->size();
      std::vector<bool> p_seen(p_size, false);

      for (unsigned nf = 0; nf < m_size; ++nf)
	{
	  unsigned np;
	  const formula* mnth = m->nth(nf);
	  for (np = 0; np < p_size; ++np)
	    {
	      if (equals(p->nth(np), mnth))
		{
		  p_seen[np] = true;
		  break;
		}
	    }
	  // We we haven't found mnth in any child of p, then
	  // the two formulas aren't equal.
	  if (np == p_size)
	    return;
	}
      // At this point, we have found all children of m' in children
      // of `p'.  That doesn't means that both formula are equal.
      // Condider m = And(a, b, c) against p = And(c, d, a, b).
      // We should now check if any unmarked (accodring to p_seen)
      // child of `p' has an counterpart in `m'.  Because `m' might
      // contain duplicate children, its faster to test that
      // unmarked children of `p' have a counterpart in marked children
      // of `p'.
      for (unsigned np = 0; np < p_size; ++np)
	{
	  // Consider only unmarked children.
	  if (p_seen[np])
	    continue;

	  // Compare with marked children.
	  unsigned np2;
	  const formula *pnth = p->nth(np);
	  for (np2 = 0; np2 < p_size; ++np2)
	    if (p_seen[np2] && equals(p->nth(np2), pnth))
	      break;

	  // No match?  Too bad.
	  if (np2 == p_size)
	    return;
	}

      // The two formulas match.
      result_ = true;
    }


    bool
    equals(const formula* f1, const formula* f2)
    {
      equals_visitor v(f1);
      f2->accept(v);
      return v.result();
    }
  }
}
