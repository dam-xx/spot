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
	    // X(true) = true
	    // X(false) = false
	    if (dynamic_cast<constant*>(result_))
	      return;

	    // XGF(f) = GF(f)
	    if (is_GF(result_))
	      return;

	    // X(f1 & GF(f2)) = X(f1) & GF(F2)
	    // X(f1 | GF(f2)) = X(f1) | GF(F2)
	    mo = dynamic_cast<multop*>(result_);
	    if (mo && mo->size() == 2)
	      {
		// FIXME: This is incomplete.  It should be done for
		// multops of any size.
		if (is_GF(mo->nth(0)))
		  {
		    multop::vec* res = new multop::vec;
		    res->push_back
		      (unop::instance(unop::F,
				      basic_reduce_form(mo->nth(1))));
		    res->push_back(basic_reduce_form(mo->nth(0)));
		    result_ = multop::instance(mo->op(), res);
		    destroy(mo);
		    return;
		  }
		if (is_GF(mo->nth(1)))
		  {
		    multop::vec* res = new multop::vec;
		    res->push_back
		      (unop::instance(unop::F,
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
	    // F(true) = true
	    // F(false) = false
	    if (dynamic_cast<constant*>(result_))
	      return;

	    // FX(a) = XF(a)
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

	    // F(f1 & GF(f2)) = F(f1) & GF(F2)
	    mo = dynamic_cast<multop*>(result_);
	    if (mo && mo->op() == multop::And
		// FIXME: This is incomplete.  It should be done for
		// "And"s of any size.
		&& mo->size() == 2)
	      {
		if (is_GF(mo->nth(0)))
		  {
		    multop::vec* res = new multop::vec;
		    res->push_back
		      (unop::instance(unop::F,
				      basic_reduce_form(mo->nth(1))));
		    res->push_back(basic_reduce_form(mo->nth(0)));
		    result_ = multop::instance(mo->op(), res);
		    destroy(mo);
		    return;
		  }
		if (is_GF(mo->nth(1)))
		  {
		    multop::vec* res = new multop::vec;
		    res->push_back
		      (unop::instance(unop::F,
				      basic_reduce_form(mo->nth(0))));
		    res->push_back(basic_reduce_form(mo->nth(1)));
		    result_ = multop::instance(mo->op(), res);
		    destroy(mo);
		    return;
		  }
	      }


	    result_ = unop::instance(unop::F, result_);
	    return;

	  case unop::G:
	    // G(true) = true
	    // G(false) = false
	    if (dynamic_cast<constant*>(result_))
	      return;

	    // G(a R b) = G(b)
	    bo = dynamic_cast<binop*>(result_);
	    if (bo && bo->op() == binop::R)
	      {
		result_ = unop::instance(unop::G,
					 basic_reduce_form(bo->second()));
		destroy(bo);
		return;
	      }

	    // GX(a) = XG(a)
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

	    // G(f1 | GF(f2)) = G(f1) | GF(F2)
	    mo = dynamic_cast<multop*>(result_);
	    if (mo && mo->op() == multop::Or
		// FIXME: This is incomplete.  It should be done for
		// "Or"s of any size.
		&& mo->size() == 2)
	      {
		if (is_GF(mo->nth(0)))
		  {
		    multop::vec* res = new multop::vec;
		    res->push_back
		      (unop::instance(unop::F,
				      basic_reduce_form(mo->nth(1))));
		    res->push_back(basic_reduce_form(mo->nth(0)));
		    result_ = multop::instance(mo->op(), res);
		    destroy(mo);
		    return;
		  }
		if (is_GF(mo->nth(1)))
		  {
		    multop::vec* res = new multop::vec;
		    res->push_back
		      (unop::instance(unop::F,
				      basic_reduce_form(mo->nth(0))));
		    res->push_back(basic_reduce_form(mo->nth(1)));
		    result_ = multop::instance(mo->op(), res);
		    destroy(mo);
		    return;
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

	    // a U false = false
	    // a U true = true
	    if (dynamic_cast<constant*>(f2))
	      {
		result_ = f2;
		return;
	      }

	    f1 = basic_reduce_form(f1);

	    // X(a) U X(b) = X(a U b)
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

	    // a R false = false
	    // a R true = true
	    if (dynamic_cast<constant*>(f2))
	      {
		result_ = f2;
		return;
	      }

	    f1 = basic_reduce_form(f1);

	    // X(a) R X(b) = X(a R b)
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
		unop* uo = dynamic_cast<unop*>(*i);
		binop* bo = dynamic_cast<binop*>(*i);
		if (uo)
		  {
		    if (uo && uo->op() == unop::X)
		      {
			// Xa & Xb = X(a & b)
			tmpX->push_back(basic_reduce_form(uo->child()));
		      }
		    else if (is_FG(*i))
		      {
			// FG(a) & FG(b) = FG(a & b)
			unop* uo2 = dynamic_cast<unop*>(uo->child());
			tmpFG->push_back(basic_reduce_form(uo2->child()));
		      }
		    else
		      {
			tmpOther->push_back(basic_reduce_form(*i));
		      }
		  }
		else if (bo)
		  {
		    if (bo->op() == binop::U)
		      {
			// (a U b) & (c U b) = (a & c) U b
			ftmp = dynamic_cast<binop*>(*i)->second();
			tmpUright = new multop::vec;
			for (multop::vec::iterator j = i; j != res->end(); j++)
			  {
			    if (!*j)
			      continue;
			    binop* bo2 = dynamic_cast<binop*>(*j);
			    if (bo2 && bo2->op() == binop::U
				&& ftmp == bo2->second())
			      {
				tmpUright
				  ->push_back(basic_reduce_form(bo2->first()));
				if (j != i)
				  {
				    destroy(*j);
				    *j = 0;
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
		      }
		    else if (bo->op() == binop::R)
		      {
			// (a R b) & (a R c) = a R (b & c)
			ftmp = dynamic_cast<binop*>(*i)->first();
			tmpRright = new multop::vec;
			for (multop::vec::iterator j = i; j != res->end(); j++)
			  {
			    if (!*j)
			      continue;
			    binop* bo2 = dynamic_cast<binop*>(*j);
			    if (bo2 && bo2->op() == binop::R
				&& ftmp == bo2->first())
			      {
				tmpRright
				  ->push_back(basic_reduce_form(bo2->second()));
				if (j != i)
				  {
				    destroy(*j);
				    *j = 0;
				  }
			      }
			  }
			tmpR
			  ->push_back(binop::instance(binop::R,
						      basic_reduce_form(ftmp),
						      multop::
						      instance(multop::And,
							       tmpRright)));
		      }
		    else
		      {
			tmpOther->push_back(basic_reduce_form(*i));
		      }
		  }
		else
		  {
		    tmpOther->push_back(basic_reduce_form(*i));
		  }
		destroy(*i);
	      }

	    delete tmpGF;
	    tmpGF = 0;

	    break;

	  case multop::Or:

	    for (multop::vec::iterator i = res->begin(); i != res->end(); i++)
	      {
		if (!*i)
		  continue;
		unop* uo = dynamic_cast<unop*>(*i);
		binop* bo = dynamic_cast<binop*>(*i);
		if (uo)
		  {
		    if (uo && uo->op() == unop::X)
		      {
			// Xa | Xb = X(a | b)
			tmpX->push_back(basic_reduce_form(uo->child()));
		      }
		    else if (is_GF(*i))
		      {
			// GF(a) | GF(b) = GF(a | b)
			unop* uo2 = dynamic_cast<unop*>(uo->child());
			tmpGF->push_back(basic_reduce_form(uo2->child()));
		      }
		    else
		      {
			tmpOther->push_back(basic_reduce_form(*i));
		      }
		  }
		else if (bo)
		  {
		    if (bo->op() == binop::U)
		      {
			// (a U b) | (a U c) = a U (b | c)
			ftmp = bo->first();
			tmpUright = new multop::vec;
			for (multop::vec::iterator j = i; j != res->end(); j++)
			  {
			    if (!*j)
			      continue;
			    binop* bo2 = dynamic_cast<binop*>(*j);
			    if (bo2 && bo2->op() == binop::U
				&& ftmp == bo2->first())
			      {
				tmpUright
				  ->push_back(basic_reduce_form(bo2->second()));
				if (j != i)
				  {
				    destroy(*j);
				    *j = 0;
				  }
			      }
			  }
			tmpU->push_back(binop::instance(binop::U,
							basic_reduce_form(ftmp),
							multop::
							instance(multop::Or,
								 tmpUright)));
		      }
		    else if (bo->op() == binop::R)
		      {
			// (a R b) | (c R b) = (a | c) R b
			ftmp = dynamic_cast<binop*>(*i)->second();
			tmpRright = new multop::vec;
			for (multop::vec::iterator j = i; j != res->end(); j++)
			  {
			    if (!*j)
			      continue;
			    binop* bo2 = dynamic_cast<binop*>(*j);
			    if (bo2 && bo2->op() == binop::R
				&& ftmp == bo2->second())
			      {
				tmpRright
				  ->push_back(basic_reduce_form(bo->first()));
				if (j != i)
				  {
				    destroy(*j);
				    *j = 0;
				  }
			      }
			  }
			tmpR
			  ->push_back(binop::instance(binop::R,
						      multop::
						      instance(multop::Or,
							       tmpRright),
						      basic_reduce_form(ftmp)));
		      }
		    else
		      {
			tmpOther->push_back(basic_reduce_form(*i));
		      }
		  }
		else
		  {
		    tmpOther->push_back(basic_reduce_form(*i));
		  }
		destroy(*i);
	      }

	    delete tmpFG;
	    tmpFG = 0;

	    break;
	  }

	res->clear();
	delete res;

	if (tmpX->size())
	  tmpOther->push_back(unop::instance(unop::X,
					     multop::instance(mo->op(),
							      tmpX)));
	else
	  delete tmpX;

	if (tmpU->size())
	  tmpOther->push_back(multop::instance(mo->op(), tmpU));
	else
	  delete tmpU;

	if (tmpR->size())
	  tmpOther->push_back(multop::instance(mo->op(), tmpR));
	else
	  delete tmpR;

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
