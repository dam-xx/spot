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

    bool
    is_GF(const formula* f)
    {
      const unop* op = dynamic_cast<const unop*>(f);
      if (op && op->op() == unop::G)
	{
	  const unop* opchild = dynamic_cast<const unop*>(op->child());
	  if (opchild && opchild->op() == unop::F)
	    return true;
	}
      return false;
    }

    bool
    is_FG(const formula* f)
    {
      const unop* op = dynamic_cast<const unop*>(f);
      if (op && op->op() == unop::F)
	{
	  const unop* opchild = dynamic_cast<const unop*>(op->child());
	  if (opchild && opchild->op() == unop::G)
	    return true;
	}
      return false;
    }

    node_type_form_visitor::node_type_form_visitor(){}

    node_type_form_visitor::type
    node_type_form_visitor::result() const { return result_;}

    void
    node_type_form_visitor::visit(const atomic_prop*)
    {
      result_ = node_type_form_visitor::Atom;
    }

    void
    node_type_form_visitor::visit(const constant*)
    {
      result_ = node_type_form_visitor::Const;
    }

    void
    node_type_form_visitor::visit(const unop*)
    {
      result_ = node_type_form_visitor::Unop;
    }

    void
    node_type_form_visitor::visit(const binop*)
    {
      result_ = node_type_form_visitor::Binop;
    }

    void
    node_type_form_visitor::visit(const multop*)
    {
      result_ = node_type_form_visitor::Multop;
    }

    node_type_form_visitor::type
    node_type(const formula* f)
    {
      node_type_form_visitor v;
      assert(f != NULL);
      const_cast<formula*>(f)->accept(v);
      return v.result();
    }

    class form_eventual_universal_visitor : public const_visitor
    {
    public:

      form_eventual_universal_visitor()
	: eventual(false), universal(false)
      {
      }

      virtual
      ~form_eventual_universal_visitor()
      {
      }

      bool
      is_eventual() const
      {
	return eventual;
      }

      bool
      is_universal() const
      {
	return universal;
      }

      void
      visit(const atomic_prop*)
      {
      }

      void
      visit(const constant*)
      {
      }

      void
      visit(const unop* uo)
      {
	const formula* f1 = uo->child();
	switch (uo->op())
	  {
	  case unop::Not:
	  case unop::X:
	    eventual = recurse_ev(f1);
	    universal = recurse_un(f1);
	    return;
	  case unop::F:
	    eventual = true;
	    return;
	  case unop::G:
	    universal = true;
	    return;
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(const binop* bo)
      {
	const formula* f1 = bo->first();
	switch (bo->op())
	  {
	  case binop::Xor:
	  case binop::Equiv:
	  case binop::Implies:
	    return;
	  case binop::U:
	    if (f1 == constant::true_instance())
	      eventual = true;
	    return;
	  case binop::R:
	    if (f1 == constant::false_instance())
	      universal = true;
	    return;
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(const multop* mo)
      {
	unsigned mos = mo->size();

	eventual = true;
	universal = true;
	for (unsigned i = 0; i < mos; ++i)
	  if (!recurse_ev(mo->nth(i)))
	    {
	      eventual = false;
	      break;
	    }
	for (unsigned i = 0; i < mos; ++i)
	  if (!recurse_un(mo->nth(i)))
	    {
	      universal = false;
	      break;
	    }
      }

      bool
      recurse_ev(const formula* f)
      {
	form_eventual_universal_visitor v;
	const_cast<formula*>(f)->accept(v);
	return v.is_eventual();
      }

      bool
      recurse_un(const formula* f)
      {
	form_eventual_universal_visitor v;
	const_cast<formula*>(f)->accept(v);
	return v.is_universal();
      }

    protected:
      bool eventual;
      bool universal;
    };


    bool is_eventual(const formula* f)
    {
      form_eventual_universal_visitor v;
      const_cast<formula*>(f)->accept(v);
      return v.is_eventual();
    }

    bool is_universal(const formula* f)
    {
      form_eventual_universal_visitor v;
      const_cast<formula*>(f)->accept(v);
      return v.is_universal();
    }

    /////////////////////////////////////////////////////////////////////////

    class inf_form_right_recurse_visitor : public const_visitor
    {
    public:

      inf_form_right_recurse_visitor(const formula *f)
	: result_(false), f(f)
      {
      }

      virtual
      ~inf_form_right_recurse_visitor()
      {
      }

      int
      result() const
      {
	return result_;
      }

      void
      visit(const atomic_prop* ap)
      {
	if (dynamic_cast<const atomic_prop*>(f) == ap)
	  result_ = true;
      }

      void
      visit(const constant* c)
      {
	switch (c->val())
	  {
	  case constant::True:
	    result_ = true;
	    return;
	  case constant::False:
	    result_ = false;
	    return;
	  }
      }

      void
      visit(const unop* uo)
      {
	const formula* f1 = uo->child();
	const formula* tmp = NULL;
	switch (uo->op())
	  {
	  case unop::Not:
	    if (uo == f)
	      result_ = true;
	    return;
	  case unop::X:
	    {
	      const unop* op = dynamic_cast<const unop*>(f);
	      if (op && op->op() == unop::X)
		result_ = inf_form(op->child(), f1);
	    }
	    return;
	  case unop::F:
	    /* F(a) = true U a */
	    result_ = inf_form(f, f1);
	    return;
	  case unop::G:
	    /* G(a) = false R a */
	    tmp = constant::false_instance();
	    if (inf_form(f, tmp))
	      result_ = true;
	    spot::ltl::destroy(tmp);
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
	  case binop::Equiv:
	  case binop::Implies:
	    return;
	  case binop::U:
	    if (inf_form(f, f2))
	      result_ = true;
	    return;
	  case binop::R:
	    if (inf_form(f, f1) && inf_form(f, f2))
	      result_ = true;
	    return;
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(const multop* mo)
      {
	multop::type op = mo->op();
	unsigned mos = mo->size();
	switch (op)
	  {
	  case multop::And:
	    for (unsigned i = 0; i < mos; ++i)
	      if (!inf_form(f, mo->nth(i)))
		return;
	    result_ = true;
	    break;
	  case multop::Or:
	    for (unsigned i = 0; i < mos && !result_; ++i)
	      if (inf_form(f, mo->nth(i)))
		result_ = true;
	    break;
	  }
      }

      bool
      recurse(const formula* f1, const formula* f2)
      {
	if (f1 == f2)
	  return true;
	inf_form_right_recurse_visitor v(f2);
	const_cast<formula*>(f1)->accept(v);
	return v.result();
      }

    protected:
      bool result_; /* true if f < f1, false otherwise. */
      const formula* f;
    };

    /////////////////////////////////////////////////////////////////////////

    class inf_form_left_recurse_visitor : public const_visitor
    {
    public:

      inf_form_left_recurse_visitor(const formula *f)
	: result_(false), f(f)
      {
      }

      virtual
      ~inf_form_left_recurse_visitor()
      {
      }

      bool
      special_case(const formula* f2)
      {
	const binop* fb = dynamic_cast<const binop*>(f);
	const binop* f2b = dynamic_cast<const binop*>(f2);
	if (fb && f2b && fb->op() == f2b->op()
	    && inf_form(f2b->first(), fb->first())
	    && inf_form(f2b->second(), fb->second()))
	  return true;
	return false;
      }

      int
      result() const
      {
	return result_;
      }

      void
      visit(const atomic_prop* ap)
      {
	inf_form_right_recurse_visitor v(ap);
	const_cast<formula*>(f)->accept(v);
	result_ = v.result();
      }

      void
      visit(const constant* c)
      {
	inf_form_right_recurse_visitor v(c);
	switch (c->val())
	  {
	  case constant::True:
	    const_cast<formula*>(f)->accept(v);
	    result_ = v.result();
	    return;
	  case constant::False:
	    result_ = true;
	    return;
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(const unop* uo)
      {
	const formula* f1 = uo->child();
	inf_form_right_recurse_visitor v(uo);
	switch (uo->op())
	  {
	  case unop::Not:
	    if (uo == f)
	      result_ = true;
	    return;
	  case unop::X:
	    {
	      const unop* op = dynamic_cast<const unop*>(f);
	      if (op && op->op() == unop::X)
		result_ = inf_form(f1, op->child());
	    }
	    return;
	  case unop::F:
	    {
	      /* F(a) = true U a */
	      const formula* tmp = binop::instance(binop::U,
						   constant::true_instance(),
						   clone(f1));
	      if (special_case(tmp))
		{
		  result_ = true;
		  spot::ltl::destroy(tmp);
		  return;
		}
	      if (inf_form(tmp, f))
		result_ = true;
	      spot::ltl::destroy(tmp);
	      return;
	    }
	  case unop::G:
	    {
	      /* F(a) = false R a */
	      const formula* tmp = binop::instance(binop::R,
						   constant::false_instance(),
						   clone(f1));
	      if (special_case(tmp))
		{
		  result_ = true;
		  spot::ltl::destroy(tmp);
		  return;
		}
	      if (inf_form(f1, f))
		result_ = true;
	      spot::ltl::destroy(tmp);
	      return;
	    }
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(const binop* bo)
      {
	if (special_case(bo))
	  {
	    result_ = true;
	    return;
	  }

	const formula* f1 = bo->first();
	const formula* f2 = bo->second();
	switch (bo->op())
	  {
	  case binop::Xor:
	  case binop::Equiv:
	  case binop::Implies:
	    return;
	  case binop::U:
	    if (inf_form(f1, f) && inf_form(f2, f))
	      result_ = true;
	    return;
	  case binop::R:
	    if (inf_form(f2, f))
	      result_ = true;
	    return;
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(const multop* mo)
      {
	multop::type op = mo->op();
	unsigned mos = mo->size();
	switch (op)
	  {
	  case multop::And:
	    for (unsigned i = 0; (i < mos) && !result_; ++i)
	      if (inf_form(mo->nth(i), f))
		result_ = true;
	    break;
	  case multop::Or:
	    for (unsigned i = 0; i < mos; ++i)
	      if (!inf_form(mo->nth(i), f))
		return;
	    result_ = true;
	    break;
	  }
      }

    protected:
      bool result_; /* true if f1 < f, 1 otherwise. */
      const formula* f;
    };

    bool
    inf_form(const formula* f1, const formula* f2)
    {
      /* f1 and f2 are unabbreviated */
      if (f1 == f2)
	return true;
      inf_form_left_recurse_visitor v1(f2);
      inf_form_right_recurse_visitor v2(f1);


      if (f2 == constant::true_instance()
	  || f1 == constant::false_instance())
	return true;

      const_cast<formula*>(f1)->accept(v1);
      if (v1.result())
	return true;

      const_cast<formula*>(f2)->accept(v2);
      if (v2.result())
	return true;

      return false;
    }

    bool infneg_form(const formula* f1, const formula* f2, int n)
    {
      const formula* ftmp1;
      const formula* ftmp2;
      const formula* ftmp3 = unop::instance(unop::Not,
					    (!n) ? clone(f1) : clone(f2));
      const formula* ftmp4 = spot::ltl::negative_normal_form((!n) ? f2 : f1);
      const formula* ftmp5;
      const formula* ftmp6;
      bool result;

      ftmp2 = spot::ltl::unabbreviate_logic(ftmp3);
      ftmp1 = spot::ltl::negative_normal_form(ftmp2);

      ftmp5 = spot::ltl::unabbreviate_logic(ftmp4);
      ftmp6 = spot::ltl::negative_normal_form(ftmp5);

      if (n == 0)
	result = inf_form(ftmp1, ftmp6);
      else
	result = inf_form(ftmp6, ftmp1);

      spot::ltl::destroy(ftmp1);
      spot::ltl::destroy(ftmp2);
      spot::ltl::destroy(ftmp3);
      spot::ltl::destroy(ftmp4);
      spot::ltl::destroy(ftmp5);
      spot::ltl::destroy(ftmp6);

      return result;
    }
  }
}
