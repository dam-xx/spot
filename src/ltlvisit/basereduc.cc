// Copyright (C) 2003  Laboratoire d'Informatique de Paris 6 (LIP6),
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
	    if (is_GF(result_)) return;

	    /* X(f1 & GF(f2)) = X(f1) & GF(F2) */
	    /* X(f1 | GF(f2)) = X(f1) | GF(F2) */
	    if (node_type(result_) == (node_type_form_visitor::Multop)) {
		if (spot::ltl::nb_term_multop(result_) == 2) {
		  if (is_GF(((multop*)result_)->nth(0)) || 
		      is_FG(((multop*)result_)->nth(0))) {
		    multop::vec* res = new multop::vec;
		    res->push_back(unop::instance(unop::F, basic_reduce_form(((multop*)result_)->nth(1))));
		    res->push_back(basic_reduce_form(((multop*)result_)->nth(0)));
		    f = result_;
		    result_ = multop::instance(((multop*)(result_))->op(), res);
		    spot::ltl::destroy(f);
		    return;
		  }
		  if (is_GF(((multop*)result_)->nth(1)) || 
		      is_FG(((multop*)result_)->nth(1))) {
		    multop::vec* res = new multop::vec;
		    res->push_back(unop::instance(unop::F, basic_reduce_form(((multop*)result_)->nth(0))));
		    res->push_back(basic_reduce_form(((multop*)result_)->nth(1)));
		    f = result_;
		    result_ = multop::instance(((multop*)(result_))->op(), res);
		    spot::ltl::destroy(f);
		    return;
		  }
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
	    if (node_type(result_) == (node_type_form_visitor::Unop))
	      if ( ((unop*)(result_))->op() == unop::X ){
		f = result_;
		result_ = unop::instance(unop::X,
					 unop::instance(unop::F,
							basic_reduce_form(((unop*)(result_))->child()) ));
		spot::ltl::destroy(f);
		return;
	      }
	    
	    /* F(f1 & GF(f2)) = F(f1) & GF(F2) */
	    if (node_type(result_) == (node_type_form_visitor::Multop)) {
	      if ( ((multop*)(result_))->op() == multop::And ) {
		if (spot::ltl::nb_term_multop(result_) == 2) {
		  if (is_GF(((multop*)result_)->nth(0)) || 
		      is_FG(((multop*)result_)->nth(0))) {
		    multop::vec* res = new multop::vec;
		    res->push_back(unop::instance(unop::F, basic_reduce_form(((multop*)result_)->nth(1))));
		    res->push_back(basic_reduce_form(((multop*)result_)->nth(0)));
		    spot::ltl::destroy(result_);
		    result_ = multop::instance(multop::And, res);
		    return;
		  }
		  if (is_GF(((multop*)result_)->nth(1)) || 
		      is_FG(((multop*)result_)->nth(1))) {
		    multop::vec* res = new multop::vec;
		    res->push_back(unop::instance(unop::F, basic_reduce_form(((multop*)result_)->nth(0))));
		    res->push_back(basic_reduce_form(((multop*)result_)->nth(1)));
		    spot::ltl::destroy(result_);
		    result_ = multop::instance(multop::And, res);
		    return;
		  }
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
	    if (node_type(result_) == node_type_form_visitor::Binop)
	      if ( ((binop*)(result_))->op() == binop::R ){
		f = result_;
		result_ = unop::instance(unop::G, basic_reduce_form(((binop*)(result_))->second()));
		spot::ltl::destroy(f);
		return;
	      }
	    
	    /* GX(a) = XG(a) */
	    if (node_type(result_) == (node_type_form_visitor::Unop))
	      if ( ((unop*)(result_))->op() == unop::X ){
		f = result_;
		result_ = unop::instance(unop::X,
					 unop::instance(unop::G,
							basic_reduce_form(((unop*)(result_))->child()) ));
		spot::ltl::destroy(f);
		return;
	      }

	    /* G(f1 | GF(f2)) = G(f1) | GF(F2) */
	    if (node_type(result_) == (node_type_form_visitor::Multop)) {
	      if ( ((multop*)(f))->op() == multop::Or ) {
		if (spot::ltl::nb_term_multop(result_) == 2) {
		  if (is_GF(((multop*)result_)->nth(0)) || 
		      is_FG(((multop*)result_)->nth(0))) {
		    multop::vec* res = new multop::vec;
		    res->push_back(unop::instance(unop::G, basic_reduce_form(((multop*)result_)->nth(1))));
		    res->push_back(basic_reduce_form(((multop*)result_)->nth(0)));
		    spot::ltl::destroy(result_);
		    result_ = multop::instance(multop::Or, res);
		    return;
		  }
		  if (is_GF(((multop*)result_)->nth(1)) || 
		      is_FG(((multop*)result_)->nth(1))) {
		    multop::vec* res = new multop::vec;
		    res->push_back(unop::instance(unop::G, basic_reduce_form(((multop*)result_)->nth(0))));
		    res->push_back(basic_reduce_form(((multop*)result_)->nth(1)));
		    spot::ltl::destroy(result_);
		    result_ = multop::instance(multop::Or, res);
		    return;
		  }
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
	    if (node_type(f2) == (node_type_form_visitor::Const)) {
	      result_ = f2;
	      return;
	    }

	    f1 = basic_reduce_form(f1);

	    /* X(a) U X(b) = X(a U b) */
	    if ( node_type(f1) == (node_type_form_visitor::Unop)
		 && node_type(f2) == (node_type_form_visitor::Unop)) {
	      if ( (((unop*)(f1))->op() == unop::X) 
		   && (((unop*)(f2))->op() == unop::X) ) {
		ftmp = binop::instance(binop::U,
				       basic_reduce_form(((unop*)(f1))->child()),
				       basic_reduce_form(((unop*)(f2))->child()));
		result_ = unop::instance(unop::X, basic_reduce_form(ftmp));
		spot::ltl::destroy(f1);
		spot::ltl::destroy(f2);
		spot::ltl::destroy(ftmp);
		return;
	      }
	    }
	    result_ = binop::instance(binop::U, f1, f2);
	    return;

	  case binop::R: 
	    f2 = basic_reduce_form(f2);

	    /* a R false = false 
	       a R true = true */
	    if (node_type(f2) == (node_type_form_visitor::Const)) {
	      result_ = f2;
	      return;
	    }

	    f1 = basic_reduce_form(f1);
	    
	    /* X(a) R X(b) = X(a R b) */
	    if ( node_type(f1) == (node_type_form_visitor::Unop)
		 && node_type(f2) == (node_type_form_visitor::Unop)) {
	      if ( (((unop*)(f1))->op() == unop::X) 
		   && (((unop*)(f2))->op() == unop::X) ) {
		ftmp = binop::instance(bo->op(),
				       basic_reduce_form(((unop*)(f1))->child()),
				       basic_reduce_form(((unop*)(f2))->child()));
		result_ = unop::instance(unop::X, basic_reduce_form(ftmp));
		spot::ltl::destroy(f1);
		spot::ltl::destroy(f2);
		spot::ltl::destroy(ftmp);
		return;
	      }
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
	if (mo == NULL);
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

	    for (multop::vec::iterator i = res->begin(); i != res->end(); i++) {
	      if (*i == NULL) continue;
	      switch (node_type(*i)) {

	      case node_type_form_visitor::Unop: 

		/* Xa & Xb = X(a & b)*/
		if (((unop*)(*i))->op() == unop::X) {
		  tmpX->push_back( spot::ltl::basic_reduce_form(((unop*)(*i))->child()) );
		  break;
		} 

		/* FG(a) & FG(b) = FG(a & b)*/
		if (is_FG(*i)) {
		  tmpFG->push_back( spot::ltl::basic_reduce_form( ((unop*)((unop*)(*i))->child())->child() ) );
		  break;
		}
		tmpOther->push_back(spot::ltl::basic_reduce_form(*i));
		break;
		/* A GERER !!!!!!!!!!!!!!!!!!!!!!!
		   case node_type_form_visitor::Const:
		   tmpX->push_back( spot::ltl::clone((*i)) );
		   break;
		*/

	      case node_type_form_visitor::Binop:

		/* (a U b) & (c U b) = (a & c) U b */
		if (((binop*)(*i))->op() == binop::U) {
		  ftmp = ((binop*)(*i))->second();
		  tmpUright = new multop::vec;
		  for (multop::vec::iterator j = i; j != res->end(); j++) 
		    {
		      if (*j == NULL) continue;
		      if (node_type(*j) == node_type_form_visitor::Binop) 
			{
			  if (((binop*)(*j))->op() == binop::U) 
			    {
			      if (ftmp == ((binop*)(*j))->second()) 
				{
				  tmpUright->push_back(spot::ltl::basic_reduce_form(((binop*)(*j))->first() ));
				  if (j != i) {
				    spot::ltl::destroy(*j);
				    *j = NULL;
				  }
				}
			    }
			}
		    }
		  tmpU->push_back(binop::instance(binop::U,
						  multop::instance(multop::And,tmpUright),
						  basic_reduce_form(ftmp)));
		  break;
		}
		
		/* (a R b) & (a R c) = a R (b & c) */
		if (((binop*)(*i))->op() == binop::R) {
		  ftmp = ((binop*)(*i))->first();
		  tmpRright = new multop::vec;
		  for (multop::vec::iterator j = i; j != res->end(); j++) 
		    {
		      if (*j == NULL) continue;
		      if (node_type(*j) == node_type_form_visitor::Binop) 
			{
			  if (((binop*)(*j))->op() == binop::R) 
			    {
			      if (ftmp == ((binop*)(*j))->first()) 
				{
				  tmpRright->push_back(spot::ltl::basic_reduce_form(((binop*)(*j))->second() ));
				  if (j != i) {
				    spot::ltl::destroy(*j);
				    *j = NULL;
				  }
				}
			    }
			}
		    }
		  tmpR->push_back(binop::instance(binop::R,
						  basic_reduce_form(ftmp),
						  multop::instance(multop::And,tmpRright)));
		  break;
		}
		tmpOther->push_back(spot::ltl::basic_reduce_form(*i));
		break;
	      default: 
		tmpOther->push_back(spot::ltl::basic_reduce_form(*i));
		break;
	      }
	      spot::ltl::destroy(*i);
	    }
	    
	    delete(tmpGF);
	    tmpGF = NULL;

	    break;
	    
	  case multop::Or:
	    
	    for (multop::vec::iterator i = res->begin(); i != res->end(); i++) {
	      if (*i == NULL) continue;
	      switch (node_type(*i)) {
		
	      case node_type_form_visitor::Unop: 

		/* Xa | Xb = X(a | b)*/
		if (((unop*)(*i))->op() == unop::X) {
		  tmpX->push_back( spot::ltl::basic_reduce_form(((unop*)(*i))->child()) );
		  break;
		}

		/* GF(a) & GF(b) = GF(a & b)*/
		if (is_GF(*i)) {
		  tmpGF->push_back( spot::ltl::basic_reduce_form( ((unop*)((unop*)(*i))->child())->child() ) );
		  break;
		}
		tmpOther->push_back(spot::ltl::basic_reduce_form(*i));
		break;

	      case node_type_form_visitor::Binop:
		
		/* (a U b) | (a U c) = a U (b | c) */
		if (((binop*)(*i))->op() == binop::U) {
		  ftmp = ((binop*)(*i))->first();
		  tmpUright = new multop::vec;
		  for (multop::vec::iterator j = i; j != res->end(); j++) 
		    {
		      if (*j == NULL) continue;
		      if (node_type(*j) == node_type_form_visitor::Binop) 
			{
			  if (((binop*)(*j))->op() == binop::U) 
			    {
			      if (ftmp == ((binop*)(*j))->first()) 
				{
				  tmpUright->push_back(spot::ltl::basic_reduce_form(((binop*)(*j))->second() ));
				  if (j != i) {
				    spot::ltl::destroy(*j);
				    *j = NULL;
				  }
				}
			    }
			}
		    }
		  tmpU->push_back(binop::instance(binop::U,
						  basic_reduce_form(ftmp),
						  multop::instance(multop::Or,tmpUright)));
		  break;
		}
		
		/* (a R b) | (c R b) = (a | c) R b */
		if (((binop*)(*i))->op() == binop::R) {
		  ftmp = ((binop*)(*i))->second();
		  tmpRright = new multop::vec;
		  for (multop::vec::iterator j = i; j != res->end(); j++) 
		    {
		      if (*j == NULL) continue;
		      if (node_type(*j) == node_type_form_visitor::Binop) 
			{
			  if (((binop*)(*j))->op() == binop::R) 
			    {
			      if (ftmp == ((binop*)(*j))->second()) 
				{
				  tmpRright->push_back(spot::ltl::basic_reduce_form(((binop*)(*j))->first() ));
				  if (j != i) {
				    spot::ltl::destroy(*j);
				    *j = NULL;
				  }
				}
			    }
			}
		    }
		  tmpR->push_back(binop::instance(binop::R,
						  multop::instance(multop::Or,tmpRright),
						  basic_reduce_form(ftmp)));
		  break;
		}
		tmpOther->push_back(spot::ltl::basic_reduce_form(*i));
		break;
	      default: 
		tmpOther->push_back(spot::ltl::basic_reduce_form(*i));
		break;
	      }
	      spot::ltl::destroy(*i);
	    }
	    
	    delete(tmpFG);
	    tmpFG = NULL;

	    break;
	  }
	
	
	res->clear();
	delete(res);
	
	if (tmpX->size())
	  tmpOther->push_back(unop::instance(unop::X,multop::instance(mo->op(),tmpX)));
	else delete(tmpX);
	
	if (tmpU->size())
	  tmpOther->push_back(multop::instance(mo->op(),tmpU));
	else delete(tmpU);
	
	if (tmpR->size())
	  tmpOther->push_back(multop::instance(mo->op(),tmpR));
	else delete(tmpR);
	    
	if (tmpGF != NULL)
	  if (tmpGF->size())
	    tmpOther->push_back(unop::instance(unop::G,
					       unop::instance(unop::F,
							      multop::instance(mo->op(),tmpGF))));
	if (tmpFG != NULL)
	  if (tmpFG->size())
	    tmpOther->push_back(unop::instance(unop::G,
					       unop::instance(unop::F,
							      multop::instance(mo->op(),tmpFG))));
	
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
