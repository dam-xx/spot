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
#include "ltlvisit/clone.hh"
#include <cassert>

#include "ltlvisit/destroy.hh"

namespace spot
{
  namespace ltl
  {
    class basic_reduce_form_visitor : public visitor
    {
    public:

      basic_reduce_form_visitor(){}

      virtual ~basic_reduce_form_visitor(){}

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
	switch (c->val())
	  {
	  case constant::True:
	    result_ = constant::true_instance();
	    return;
	  case constant::False:
	    result_ = constant::false_instance();
	    return;
	  }
      }

      void
      visit(unop* uo)
      {
	formula* f = uo->child();
	result_ = basic_reduce_form(f);
	multop* mo = NULL;
	unop* u = NULL;
	binop* bo = NULL;
	switch (uo->op())
	  {
	  case unop::Not:
	    result_ = unop::instance(unop::Not, result_);
	    return;

	  case unop::X:
	    /* X(true) = true
	       X(false) = false */
	    if (node_type(result_) == (node_type_form_visitor::Const))
	      return;

	    /* XGF(f) = GF(f) */
	    if (is_GF(result_))
	      return;

	    /* X(f1 & GF(f2)) = X(f1) & GF(F2) */
	    /* X(f1 | GF(f2)) = X(f1) | GF(F2) */
	    mo = dynamic_cast<multop*>(result_);
	    if (mo && mo->size() == 2)
	      {
		if (is_GF(mo->nth(0)))
		  {
		    multop::vec* res = new multop::vec;
		    res->push_back(unop::instance(unop::F,
						  basic_reduce_form(mo->nth(1))));
		    res->push_back(basic_reduce_form(mo->nth(0)));
		    result_ = multop::instance(mo->op(), res);
		    destroy(mo);
		    return;
		  }
		if (is_GF(mo->nth(1)))
		  {
		    multop::vec* res = new multop::vec;
		    res->push_back(unop::instance(unop::F,
						  basic_reduce_form(mo->nth(0))));
		    res->push_back(basic_reduce_form(mo->nth(1)));
		    result_ = multop::instance(mo->op(), res);
		    destroy(mo);
		    return;
		  }
	      }


	    result_ = unop::instance(unop::X, result_);
	    return;

	  case unop::F:

	    /* F(true) = true
	       F(false) = false */
	    if (node_type(result_) == (node_type_form_visitor::Const))
	      return;

	    /* FX(a) = XF(a) */
	    u = dynamic_cast<unop*>(result_);
	    if (u && u->op() == unop::X)
	      {
		result_ =
		  unop::instance(unop::X,
				 unop::instance(unop::F,
						basic_reduce_form(u->child())));
		destroy(u);
	      return;
	    }

	    /* F(f1 & GF(f2)) = F(f1) & GF(F2) */
	    mo = dynamic_cast<multop*>(result_);
	    if (mo && mo->op() == multop::And)
	      {
		if (mo->size() == 2)
		  {
		    if (is_GF(mo->nth(0)))
		      {
			multop::vec* res = new multop::vec;
			res->push_back(unop::instance(unop::F,
						      basic_reduce_form(mo->nth(1))));
			res->push_back(basic_reduce_form(mo->nth(0)));
			result_ = multop::instance(mo->op(), res);
			destroy(mo);
			return;
		      }
		    if (is_GF(mo->nth(1)))
		      {
			multop::vec* res = new multop::vec;
			res->push_back(unop::instance(unop::F,
						      basic_reduce_form(mo->nth(0))));
			res->push_back(basic_reduce_form(mo->nth(1)));
			result_ = multop::instance(mo->op(), res);
			destroy(mo);
			return;
		      }
		  }
	      }


	    result_ = unop::instance(unop::F, result_);
	    return;

	  case unop::G:

	    /* G(true) = true
	       G(false) = false */
	    if (node_type(result_) == (node_type_form_visitor::Const))
	      return;

	    /* G(a R b) = G(b) */
	    bo = dynamic_cast<binop*>(result_);
	    if (bo && bo->op() == binop::R)
	      {
		result_ = unop::instance(unop::G,
					 basic_reduce_form(bo->second()));
		destroy(bo);
		return;
	      }

	    /* GX(a) = XG(a) */
	    u = dynamic_cast<unop*>(result_);
	    if (u && u->op() == unop::X)
	      {
		result_ =
		  unop::instance(unop::X,
				 unop::instance(unop::G,
						basic_reduce_form(u->child())));
		destroy(u);
		return;
	      }

	    /* G(f1 | GF(f2)) = G(f1) | GF(F2) */
	    mo = dynamic_cast<multop*>(result_);
	    if (mo && mo->op() == multop::Or)
	      {
		if (mo->size() == 2)
		  {
		    if (is_GF(mo->nth(0)))
		      {
			multop::vec* res = new multop::vec;
			res->push_back(unop::instance(unop::F,
						      basic_reduce_form(mo->nth(1))));
			res->push_back(basic_reduce_form(mo->nth(0)));
			result_ = multop::instance(mo->op(), res);
			destroy(mo);
			return;
		      }
		    if (is_GF(mo->nth(1)))
		      {
			multop::vec* res = new multop::vec;
			res->push_back(unop::instance(unop::F,
						      basic_reduce_form(mo->nth(0))));
			res->push_back(basic_reduce_form(mo->nth(1)));
			result_ = multop::instance(mo->op(), res);
			destroy(mo);
			return;
		      }
		  }
	      }


	    result_ = unop::instance(unop::G, result_);
	    return;
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(binop* bo)
      {
	formula* f1 = bo->first();
	formula* f2 = bo->second();
	formula* ftmp = NULL;
	node_type_form_visitor v1;
	node_type_form_visitor v2;
	unop* fu1 = NULL;
	unop* fu2 = NULL;
	switch (bo->op())
	  {
	  case binop::Xor:
	    result_ = binop::instance(binop::Xor,
				      basic_reduce_form(f1),
				      basic_reduce_form(f2));
	    return;
	  case binop::Equiv:
	    result_ = binop::instance(binop::Equiv,
				      basic_reduce_form(f1),
				      basic_reduce_form(f2));
	    return;
	  case binop::Implies:
	    result_ = binop::instance(binop::Implies,
				      basic_reduce_form(f1),
				      basic_reduce_form(f2));
	    return;
	  case binop::U:
	    f2 = basic_reduce_form(f2);

	    /* a U false = false
	       a U true = true */
	    if (node_type(f2) == (node_type_form_visitor::Const))
	      {
		result_ = f2;
		return;
	      }

	    f1 = basic_reduce_form(f1);

	    /* X(a) U X(b) = X(a U b) */
	    fu1 = dynamic_cast<unop*>(f1);
	    fu2 = dynamic_cast<unop*>(f2);
	    if ((fu1 && fu2) &&
		(fu1->op() == unop::X) &&
		(fu2->op() == unop::X))
	      {
		ftmp = binop::instance(binop::U,
				       basic_reduce_form(fu1->child()),
				       basic_reduce_form(fu2->child()));
		result_ = unop::instance(unop::X, basic_reduce_form(ftmp));
		destroy(f1);
		destroy(f2);
		destroy(ftmp);
		return;
	      }

	    result_ = binop::instance(binop::U, f1, f2);
	    return;

	  case binop::R:
	    f2 = basic_reduce_form(f2);

	    /* a R false = false
	       a R true = true */
	    if (node_type(f2) == (node_type_form_visitor::Const))
	      {
		result_ = f2;
		return;
	      }

	    f1 = basic_reduce_form(f1);

	    /* X(a) R X(b) = X(a R b) */
	    fu1 = dynamic_cast<unop*>(f1);
	    fu2 = dynamic_cast<unop*>(f2);
	    if ((fu1 && fu2) &&
		(fu1->op() == unop::X) &&
		(fu2->op() == unop::X))
	      {
		ftmp = binop::instance(bo->op(),
				       basic_reduce_form(fu1->child()),
				       basic_reduce_form(fu2->child()));
		result_ = unop::instance(unop::X, basic_reduce_form(ftmp));
		destroy(f1);
		destroy(f2);
		destroy(ftmp);
		return;
	    }

	    result_ = binop::instance(bo->op(),
				      f1, f2);
	    return;
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(multop* mo)
      {
	multop::type op = mo->op();
	unsigned mos = mo->size();
	multop::vec* res = new multop::vec;

	multop::vec* tmpX = new multop::vec;
	multop::vec* tmpU = new multop::vec;
	multop::vec* tmpUright = NULL;
	multop::vec* tmpR = new multop::vec;
	multop::vec* tmpRright = NULL;
	multop::vec* tmpFG = new multop::vec;
	multop::vec* tmpGF = new multop::vec;

	multop::vec* tmpOther = new multop::vec;

	unop* uo = NULL;
	unop* uo2 = NULL;
	binop* bo = NULL;
	formula* ftmp = NULL;

	for (unsigned i = 0; i < mos; ++i)
	  res->push_back(basic_reduce_form(mo->nth(i)));


	switch (op)
	  {
	  case multop::And:

	    for (multop::vec::iterator i = res->begin(); i != res->end(); i++)
	      {
		if (*i == NULL)
		  continue;
		switch (node_type(*i))
		  {

		  case node_type_form_visitor::Unop:

		    /* Xa & Xb = X(a & b)*/
		    uo = dynamic_cast<unop*>(*i);
		    if (uo && uo->op() == unop::X)
		      {
			tmpX->push_back(basic_reduce_form(uo->child()));
			break;
		      }

		    /* FG(a) & FG(b) = FG(a & b)*/
		    if (is_FG(*i))
		      {
			uo2 = dynamic_cast<unop*>(uo->child());
			tmpFG->push_back(basic_reduce_form(uo2->child()));
			break;
		      }
		    tmpOther->push_back(basic_reduce_form(*i));
		    break;

		  case node_type_form_visitor::Binop:

		    /* (a U b) & (c U b) = (a & c) U b */
		    if (dynamic_cast<binop*>(*i)->op() == binop::U)
		      {
			ftmp = dynamic_cast<binop*>(*i)->second();
			tmpUright = new multop::vec;
			for (multop::vec::iterator j = i; j != res->end(); j++)
			  {
			    if (*j == NULL)
			      continue;
			    if ((node_type(*j) == node_type_form_visitor::Binop)
				&&
				(dynamic_cast<binop*>(*j)->op() == binop::U) &&
				(ftmp == dynamic_cast<binop*>(*j)->second()))
			      {
				bo = dynamic_cast<binop*>(*j);
				tmpUright
				  ->push_back(basic_reduce_form(bo->first()));
				if (j != i)
				  {
				    destroy(*j);
				    *j = NULL;
				  }
			      }
			  }
			tmpU
			  ->push_back(binop::instance(binop::U,
						      multop::
						      instance(multop::
							       And,
							       tmpUright),
						      basic_reduce_form(ftmp)));
			break;
		      }

		    /* (a R b) & (a R c) = a R (b & c) */
		    if (dynamic_cast<binop*>(*i)->op() == binop::R)
		      {
			ftmp = dynamic_cast<binop*>(*i)->first();
			tmpRright = new multop::vec;
			for (multop::vec::iterator j = i; j != res->end(); j++)
			  {
			    if (*j == NULL)
			      continue;
			    if ((node_type(*j) == node_type_form_visitor::Binop)
				&&
				(dynamic_cast<binop*>(*j)->op() == binop::R) &&
				(ftmp == dynamic_cast<binop*>(*j)->first()))
			      {
				bo = dynamic_cast<binop*>(*j);
				tmpRright
				  ->push_back(basic_reduce_form(bo->second()));
				if (j != i)
				  {
				    destroy(*j);
				    *j = NULL;
				  }
			      }
			  }
			tmpR
			  ->push_back(binop::instance(binop::R,
						      basic_reduce_form(ftmp),
						      multop::
						      instance(multop::And,
							       tmpRright)));
			break;
		      }
		    tmpOther->push_back(basic_reduce_form(*i));
		    break;
		  default:
		    tmpOther->push_back(basic_reduce_form(*i));
		    break;
		  }
		destroy(*i);
	      }

	    delete(tmpGF);
	    tmpGF = NULL;

	    break;

	  case multop::Or:

	    for (multop::vec::iterator i = res->begin(); i != res->end(); i++)
	      {
		if (*i == NULL)
		  continue;
		switch (node_type(*i))
		  {

		  case node_type_form_visitor::Unop:

		    /* Xa | Xb = X(a | b)*/
		    uo = dynamic_cast<unop*>(*i);
		    if (uo && uo->op() == unop::X)
		      {
			tmpX->push_back(basic_reduce_form(uo->child()));
			break;
		      }

		    /* GF(a) | GF(b) = GF(a | b)*/
		    if (is_GF(*i))
		      {
			uo2 = dynamic_cast<unop*>(uo->child());
			tmpGF->push_back(basic_reduce_form(uo2->child()));
			break;
		      }
		    tmpOther->push_back(basic_reduce_form(*i));
		    break;

		  case node_type_form_visitor::Binop:

		    /* (a U b) | (a U c) = a U (b | c) */
		    if (dynamic_cast<binop*>(*i)->op() == binop::U)
		      {
			ftmp = dynamic_cast<binop*>(*i)->first();
			tmpUright = new multop::vec;
			for (multop::vec::iterator j = i; j != res->end(); j++)
			  {
			    if (*j == NULL)
			      continue;
			    if ((node_type(*j) == node_type_form_visitor::Binop)
				&&
				(dynamic_cast<binop*>(*j)->op() == binop::U) &&
				(ftmp == dynamic_cast<binop*>(*j)->first()))
			      {
				bo = dynamic_cast<binop*>(*j);
				tmpUright
				  ->push_back(basic_reduce_form(bo->second()));
				if (j != i)
				  {
				    destroy(*j);
				    *j = NULL;
				  }
			      }
			  }
			tmpU->push_back(binop::instance(binop::U,
							basic_reduce_form(ftmp),
							multop::
							instance(multop::Or,
								 tmpUright)));
			break;
		      }

		    /* (a R b) | (c R b) = (a | c) R b */
		    if (dynamic_cast<binop*>(*i)->op() == binop::R)
		      {
			ftmp = dynamic_cast<binop*>(*i)->second();
			tmpRright = new multop::vec;
			for (multop::vec::iterator j = i; j != res->end(); j++)
			  {
			    if (*j == NULL)
			      continue;
			    if ((node_type(*j) == node_type_form_visitor::Binop)
				&&
				(dynamic_cast<binop*>(*j)->op() == binop::R) &&
				(ftmp == dynamic_cast<binop*>(*j)->second()))
			      {
				bo = dynamic_cast<binop*>(*j);
				tmpRright
				  ->push_back(basic_reduce_form(bo->first()));
				if (j != i)
				  {
				    destroy(*j);
				    *j = NULL;
				  }
			      }
			  }
			tmpR
			  ->push_back(binop::instance(binop::R,
						      multop::
						      instance(multop::Or,
							       tmpRright),
						      basic_reduce_form(ftmp)));
			break;
		      }
		    tmpOther->push_back(basic_reduce_form(*i));
		    break;
		  default:
		    tmpOther->push_back(basic_reduce_form(*i));
		    break;
		  }
		destroy(*i);
	      }

	    delete(tmpFG);
	    tmpFG = NULL;

	    break;
	  }


	res->clear();
	delete(res);

	if (tmpX->size())
	  tmpOther->push_back(unop::instance(unop::X,
					     multop::instance(mo->op(),
							      tmpX)));
	else delete(tmpX);

	if (tmpU->size())
	  tmpOther->push_back(multop::instance(mo->op(), tmpU));
	else delete(tmpU);

	if (tmpR->size())
	  tmpOther->push_back(multop::instance(mo->op(), tmpR));
	else delete(tmpR);

	if ((tmpGF != NULL) &&
	    (tmpGF->size()))
	  {
	    ftmp = unop::instance(unop::G,
				  unop::instance(unop::F,
						 multop::instance(mo->op(),
								  tmpGF)));
	    tmpOther->push_back(ftmp);
	  }
	if ((tmpFG != NULL) &&
	    (tmpFG->size()))
	  {
	    ftmp = unop::instance(unop::F,
				  unop::instance(unop::G,
						 multop::instance(mo->op(),
								  tmpFG)));
	    tmpOther->push_back(ftmp);
	  }

	result_ = multop::instance(op, tmpOther);

	return;

      }

    protected:
      formula* result_;
    };

    formula* basic_reduce_form(const formula* f)
    {
      basic_reduce_form_visitor v;
      const_cast<formula*>(f)->accept(v);
      return v.result();
    }

  }
}
