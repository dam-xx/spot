// Copyright (C) 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
// département Systèmes Répartis Coopératifs (SRC), Université Pierre
// et Marie Curie.
//
// This file is part of Spot, a model checking library.
//
// Spot is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Spot is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Spot; see the file COPYING.  If not, write to the Free
// Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#include "reducform.hh"
#include "ltlast/allnodes.hh"
#include <cassert>

#include "lunabbrev.hh"
#include "nenoform.hh"
#include "ltlvisit/destroy.hh"

namespace spot
{
  namespace ltl
  {

    class reduce_visitor : public visitor
    {
    public:

      reduce_visitor(int opt)
	: opt_(opt)
      {
      }

      virtual ~reduce_visitor()
      {
      }

      formula*
      result() const
      {
	return result_;
      }

      void
      visit(atomic_prop* ap)
      {
	formula* f = ap->ref();
	result_ = f;
      }

      void
      visit(constant* c)
      {
	result_ = c;
      }

      void
      visit(unop* uo)
      {
	result_ = recurse(uo->child());

	switch (uo->op())
	  {
	  case unop::Not:
	    result_ = unop::instance(unop::Not, result_);
	    return;

	  case unop::X:
	    result_ = unop::instance(unop::X, result_);
	    return;

	  case unop::F:
	    /* If f is a pure eventuality formula then F(f)=f.  */
	    if (!(opt_ & Reduce_Eventuality_And_Universality)
		|| !is_eventual(result_))
	      result_ = unop::instance(unop::F, result_);
	    return;

	  case unop::G:
	    /* If f is a pure universality formula then G(f)=f.  */
	    if (!(opt_ & Reduce_Eventuality_And_Universality)
		|| !is_universal(result_))
	      result_ = unop::instance(unop::G, result_);
	    return;
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(binop* bo)
      {
	formula* f2 = recurse(bo->second());

	/* If b is a pure eventuality formula then a U b = b.
	   If b is a pure universality formula a R b = b. */
	if ((opt_ & Reduce_Eventuality_And_Universality)
	    && ((is_eventual(f2) && ((bo->op()) == binop::U))
		|| (is_universal(f2) && ((bo->op()) == binop::R))))
	  {
	    result_ = f2;
	    return;
	  }
	/* case of implies */
	formula* f1 = recurse(bo->first());

	if (opt_ & Reduce_Syntactic_Implications)
	  {
	    // FIXME: These should be done only when needed.
	    bool inf = syntactic_implication(f1, f2);
	    bool infinv = syntactic_implication(f2, f1);
	    bool infnegleft = syntactic_implication_neg(f2, f1, false);
	    bool infnegright = syntactic_implication_neg(f2, f1, true);

	    switch (bo->op())
	      {
	      case binop::Xor:
	      case binop::Equiv:
	      case binop::Implies:
		break;

	      case binop::U:
		/* a < b => a U b = b */
		if (inf)
		  {
		    result_ = f2;
		    destroy(f1);
		    return;
		  }
		/* !b < a => a U b = Fb */
		if (infnegleft)
		  {
		    result_ = unop::instance(unop::F, f2);
		    destroy(f1);
		    return;
		  }
		/* a < b => a U (b U c) = (b U c) */
		{
		  binop* bo = dynamic_cast<binop*>(f2);
		  if (bo && bo->op() == binop::U
		      && syntactic_implication(f1, bo->first()))
		    {
		      result_ = f2;
		      destroy(f1);
		      return;
		    }
		}
		break;

	      case binop::R:
		/* b < a => a R b = b */
		if (infinv)
		  {
		    result_ = f2;
		    destroy(f1);
		    return;
		  }
		/* b < !a => a R b = Gb */
		if (infnegright)
		  {
		    result_ = unop::instance(unop::G, f2);
		    destroy(f1);
		    return;
		  }
		/* b < a => a R (b R c) = b R c */
		{
		  binop* bo = dynamic_cast<binop*>(f2);
		  if (bo && bo->op() == binop::R
		      && syntactic_implication(bo->first(), f1))
		    {
		      result_ = f2;
		      destroy(f1);
		      return;
		    }
		}
		break;
	      }
	  }
	result_ = binop::instance(bo->op(), f1, f2);
      }

      void
      visit(multop* mo)
      {
	formula* f1 = NULL;
	formula* f2 = NULL;
	unsigned mos = mo->size();
	multop::vec* res = new multop::vec;
	multop::vec::iterator index;
	multop::vec::iterator indextmp;

	for (unsigned i = 0; i < mos; ++i)
	  res->push_back(recurse(mo->nth(i)));

	if (opt_ & Reduce_Syntactic_Implications)
	  {
	    switch (mo->op())
	      {

	      case multop::Or:
		index = indextmp = res->begin();
		if (index != res->end())
		  {
		    f1 = *index;
		    index++;
		  }
		for (; index != res->end(); index++)
		  {
		    f2 = *index;
		    /* a < b => a + b = b */
		    if (syntactic_implication(f1, f2)) // f1 < f2
		      {
			f1 = f2;
			destroy(*indextmp);
			res->erase(indextmp);
			indextmp = index;
			index--;
		      }
		    else if (syntactic_implication(f2, f1)) // f2 < f1
		      {
			destroy(*index);
			res->erase(index);
			index--;
		      }
		  }
		break;

	      case multop::And:
		index = indextmp = res->begin();
		if (mos)
		  {
		    f1 = mo->nth(0);
		    index++;
		  }
		for (; index != res->end(); index++)
		  {
		    f2 = *index;
		    /* a < b => a & b = a */
		    if (syntactic_implication(f1, f2)) // f1 < f2
		      {
			destroy(*index);
			res->erase(index);
			index--;
		      }
		    else if (syntactic_implication(f2, f1)) // f2 < f1
		      {
			f1 = f2;
			destroy(*indextmp);
			res->erase(indextmp);
			indextmp = index;
			index--;
		      }
		  }
		break;
	      }

	    /* f1 < !f2 => f1 & f2 = false
	       !f1 < f2 => f1 | f2 = true */
	    for (index = res->begin(); index != res->end(); index++)
	      for (indextmp = res->begin(); indextmp != res->end(); indextmp++)
		if (index != indextmp
		    && syntactic_implication_neg(*index, *indextmp,
						 mo->op() != multop::Or))
		  {
		    for (multop::vec::iterator j = res->begin();
			 j != res->end(); j++)
		      destroy(*j);
		    if (mo->op() == multop::Or)
		      result_ = constant::true_instance();
		    else
		      result_ = constant::false_instance();
		    return;
		  }
	  }
	if (res->size())
	  {
	    result_ = multop::instance(mo->op(), res);
	    return;
	  }
	assert(0);
      }

      formula*
      recurse(formula* f)
      {
	return reduce(f, opt_);
      }

    protected:
      formula* result_;
      int opt_;
    };

    formula*
    reduce(const formula* f, int opt)
    {
      formula* f1;
      formula* f2;

      f1 = unabbreviate_logic(f);
      f2 = negative_normal_form(f1);
      destroy(f1);

      if (opt & Reduce_Basics)
	{
	  f1 = basic_reduce(f2);
	  destroy(f2);
	  f2 = f1;
	}
      if (opt & (Reduce_Syntactic_Implications
		 | Reduce_Eventuality_And_Universality))
	{
	  reduce_visitor v(opt);
	  f2->accept(v);
	  f1 = v.result();
	  destroy(f2);
	  f2 = f1;

	  // Run basic_reduce again.
	  //
	  // Consider `F b & g'   were g is an eventual formula rewritten
	  // as `g = F c'  Then basic_reduce with rewrite it
	  // as F(b & c).
	  if (opt & Reduce_Basics)
	    {
	      f1 = basic_reduce(f2);
	      destroy(f2);
	      f2 = f1;
	    }
	}
      return f2;
    }
  }
}
