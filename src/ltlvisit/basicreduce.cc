// Copyright (C) 2008, 2009, 2010 Laboratoire de Recherche et
// Développement de l'Epita (LRDE).
// Copyright (C) 2004, 2007 Laboratoire d'Informatique de
// Paris 6 (LIP6), département Systèmes Répartis Coopératifs (SRC),
// Université Pierre et Marie Curie.
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

#include "basicreduce.hh"
#include "ltlast/visitor.hh"
#include "ltlast/allnodes.hh"
#include <cassert>

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

    namespace
    {
      class basic_reduce_visitor: public visitor
      {
      public:

	basic_reduce_visitor(){}

	virtual ~basic_reduce_visitor(){}

	formula*
	result() const
	{
	  return result_;
	}

	void
	visit(atomic_prop* ap)
	{
	  formula* f = ap->clone();
	  result_ = f;
	}

	void
	visit(constant* c)
	{
	  result_ = c;
	}

	formula*
	param_case(multop* mo, unop::type op, multop::type op_child)
	{
	  formula* result;
	  multop::vec* res1 = new multop::vec;
	  multop::vec* resGF = new multop::vec;
	  unsigned mos = mo->size();
	  for (unsigned i = 0; i < mos; ++i)
	    if (is_GF(mo->nth(i)))
	      resGF->push_back(mo->nth(i)->clone());
	    else
	      res1->push_back(mo->nth(i)->clone());
	  mo->destroy();
	  multop::vec* res3 = new multop::vec;
	  if (!res1->empty())
	    res3->push_back(unop::instance(op,
					   multop::instance(op_child, res1)));
	  else
	    delete res1;
	  if (!resGF->empty())
	    res3->push_back(multop::instance(op_child, resGF));
	  else
	    delete resGF;
	  result = multop::instance(op_child, res3);
	  return result;
	}

	void
	visit(unop* uo)
	{
	  formula* f = uo->child();
	  result_ = basic_reduce(f);
	  multop* mo = 0;
	  unop* u = 0;
	  binop* bo = 0;
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
	      if (mo)
		{
		  result_ = param_case(mo, unop::X, mo->op());
		  return;
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
		  formula* res =
		    unop::instance(unop::X,
				   unop::instance(unop::F,
						  basic_reduce(u->child())));
		  u->destroy();
		  // FXX(a) = XXF(a) ...
		  result_ = basic_reduce(res);
		  res->destroy();
		  return;
		}

#if 0
	      // F(f1 & GF(f2)) = F(f1) & GF(F2)
	      //
	      // As is, these two formulae are translated into
	      // equivalent Büchi automata so the rewriting is
	      // useless.
	      //
	      // However when taken in a larger formulae such as F(f1
	      // & GF(f2)) | F(a & GF(b)), this rewriting used to
	      // produce (F(f1) & GF(f2)) | (F(a) & GF(b)), missing
	      // the opportunity to apply the F(E1)|F(E2) = F(E1|E2)
	      // rule which really helps the translation. F((f1 &
	      // GF(f2)) | (a & GF(b))) is indeed easier to translate.
	      //
	      // So let's not consider this rewriting rule.
	      mo = dynamic_cast<multop*>(result_);
	      if (mo && mo->op() == multop::And)
		{
		  result_ = param_case(mo, unop::F, multop::And);
		  return;
		}
#endif

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
					   basic_reduce(bo->second()));
		  bo->destroy();
		  return;
		}

	      // GX(a) = XG(a)
	      u = dynamic_cast<unop*>(result_);
	      if (u && u->op() == unop::X)
		{
		  formula* res =
		    unop::instance(unop::X,
				   unop::instance(unop::G,
						  basic_reduce(u->child())));
		  u->destroy();
		  // GXX(a) = XXG(a) ...
		  // GXF(a) = XGF(a) = GF(a) ...
		  result_ = basic_reduce(res);
		  res->destroy();
		  return;
		}

	      // G(f1 | GF(f2)) = G(f1) | GF(F2)
	      mo = dynamic_cast<multop*>(result_);
	      if (mo && mo->op() == multop::Or)
		{
		  result_ = param_case(mo, unop::G, multop::Or);
		  return;
		}

	      result_ = unop::instance(unop::G, result_);
	      return;

	    case unop::Finish:
	      result_ = unop::instance(unop::Finish, result_);
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
	  unop* fu1;
	  unop* fu2;
	  switch (bo->op())
	    {
	    case binop::Xor:
	    case binop::Equiv:
	    case binop::Implies:
	      result_ = binop::instance(bo->op(),
					basic_reduce(f1),
					basic_reduce(f2));
	      return;
	    case binop::U:
	    case binop::R:
	      f2 = basic_reduce(f2);

	      // a U false = false
	      // a U true = true
	      // a R false = false
	      // a R true = true
	      if (dynamic_cast<constant*>(f2))
		{
		  result_ = f2;
		  return;
		}

	      f1 = basic_reduce(f1);

	      // X(a) U X(b) = X(a U b)
	      // X(a) R X(b) = X(a R b)
	      fu1 = dynamic_cast<unop*>(f1);
	      fu2 = dynamic_cast<unop*>(f2);
	      if (fu1 && fu2
		  && fu1->op() == unop::X
		  && fu2->op() == unop::X)
		{
		  formula* ftmp = binop::instance(bo->op(),
						  basic_reduce(fu1->child()),
						  basic_reduce(fu2->child()));
		  result_ = unop::instance(unop::X, basic_reduce(ftmp));
		  f1->destroy();
		  f2->destroy();
		  ftmp->destroy();
		  return;
	      }

	      result_ = binop::instance(bo->op(), f1, f2);
	      return;
	    }
	  /* Unreachable code.  */
	  assert(0);
	}

	void
	visit(automatop*)
	{
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
	  multop::vec* tmpR = new multop::vec;
	  multop::vec* tmpFG = new multop::vec;
	  multop::vec* tmpF = new multop::vec;
	  multop::vec* tmpGF = new multop::vec;
	  multop::vec* tmpG = new multop::vec;

	  multop::vec* tmpOther = new multop::vec;

	  for (unsigned i = 0; i < mos; ++i)
	    res->push_back(basic_reduce(mo->nth(i)));

	  switch (op)
	    {
	    case multop::And:

	      for (multop::vec::iterator i = res->begin(); i != res->end(); i++)
		{
		  // An iteration of the loop may zero some later elements
		  // of the vector to mark them as redundant.  Skip them.
		  if (!*i)
		    continue;
		  unop* uo = dynamic_cast<unop*>(*i);
		  binop* bo = dynamic_cast<binop*>(*i);
		  if (uo)
		    {
		      if (uo->op() == unop::X)
			{
			  // Xa & Xb = X(a & b)
			  tmpX->push_back(uo->child()->clone());
			}
		      else if (is_FG(*i))
			{
			  // FG(a) & FG(b) = FG(a & b)
			  unop* uo2 = dynamic_cast<unop*>(uo->child());
			  tmpFG->push_back(uo2->child()->clone());
			}
		      else if (uo->op() == unop::G)
			{
			  // G(a) & G(b) = G(a & b)
			  tmpG->push_back(uo->child()->clone());
			}
		      else
			{
			  tmpOther->push_back((*i)->clone());
			}
		    }
		  else if (bo)
		    {
		      if (bo->op() == binop::U)
			{
			  // (a U b) & (c U b) = (a & c) U b
			  formula* ftmp = dynamic_cast<binop*>(*i)->second();
			  multop::vec* tmpUright = new multop::vec;
			  for (multop::vec::iterator j = i; j != res->end();
			       j++)
			    {
			      if (!*j)
				continue;
			      binop* bo2 = dynamic_cast<binop*>(*j);
			      if (bo2 && bo2->op() == binop::U
				  && ftmp == bo2->second())
				{
				  tmpUright
				    ->push_back(bo2->first()->clone());
				  if (j != i)
				    {
				      (*j)->destroy();
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
							ftmp->clone()));
			}
		      else if (bo->op() == binop::R)
			{
			  // (a R b) & (a R c) = a R (b & c)
			  formula* ftmp = dynamic_cast<binop*>(*i)->first();
			  multop::vec* tmpRright = new multop::vec;
			  for (multop::vec::iterator j = i; j != res->end();
			       j++)
			    {
			      if (!*j)
				continue;
			      binop* bo2 = dynamic_cast<binop*>(*j);
			      if (bo2 && bo2->op() == binop::R
				  && ftmp == bo2->first())
				{
				  tmpRright
				    ->push_back(bo2->second()->clone());
				  if (j != i)
				    {
				      (*j)->destroy();
				      *j = 0;
				    }
				}
			    }
			  tmpR
			    ->push_back(binop::instance(binop::R,
							ftmp->clone(),
							multop::
							instance(multop::And,
								 tmpRright)));
			}
		      else
			{
			  tmpOther->push_back((*i)->clone());
			}
		    }
		  else
		    {
		      tmpOther->push_back((*i)->clone());
		    }
		  (*i)->destroy();
		}
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
		      if (uo->op() == unop::X)
			{
			  // Xa | Xb = X(a | b)
			  tmpX->push_back(uo->child()->clone());
			}
		      else if (is_GF(*i))
			{
			  // GF(a) | GF(b) = GF(a | b)
			  unop* uo2 = dynamic_cast<unop*>(uo->child());
			  tmpGF->push_back(uo2->child()->clone());
			}
		      else if (uo->op() == unop::F)
			{
			  // F(a) | F(b) = F(a | b)
			  tmpF->push_back(uo->child()->clone());
			}
		      else
			{
			  tmpOther->push_back((*i)->clone());
			}
		    }
		  else if (bo)
		    {
		      if (bo->op() == binop::U)
			{
			  // (a U b) | (a U c) = a U (b | c)
			  formula* ftmp = bo->first();
			  multop::vec* tmpUright = new multop::vec;
			  for (multop::vec::iterator j = i; j != res->end();
			       j++)
			    {
			      if (!*j)
				continue;
			      binop* bo2 = dynamic_cast<binop*>(*j);
			      if (bo2 && bo2->op() == binop::U
				  && ftmp == bo2->first())
				{
				  tmpUright
				    ->push_back(bo2->second()->clone());
				  if (j != i)
				    {
				      (*j)->destroy();
				      *j = 0;
				    }
				}
			    }
			  tmpU->push_back(binop::instance(binop::U,
							  ftmp->clone(),
							  multop::
							  instance(multop::Or,
								   tmpUright)));
			}
		      else if (bo->op() == binop::R)
			{
			  // (a R b) | (c R b) = (a | c) R b
			  formula* ftmp = dynamic_cast<binop*>(*i)->second();
			  multop::vec* tmpRright = new multop::vec;
			  for (multop::vec::iterator j = i; j != res->end();
			       j++)
			    {
			      if (!*j)
				continue;
			      binop* bo2 = dynamic_cast<binop*>(*j);
			      if (bo2 && bo2->op() == binop::R
				  && ftmp == bo2->second())
				{
				  tmpRright
				    ->push_back(bo2->first()->clone());
				  if (j != i)
				    {
				      (*j)->destroy();
				      *j = 0;
				    }
				}
			    }
			  tmpR
			    ->push_back(binop::instance(binop::R,
							multop::
							instance(multop::Or,
								 tmpRright),
							ftmp->clone()));
			}
		      else
			{
			  tmpOther->push_back((*i)->clone());
			}
		    }
		  else
		    {
		      tmpOther->push_back((*i)->clone());
		    }
		  (*i)->destroy();
		}

	      break;
	    }

	  res->clear();
	  delete res;


	  if (tmpX && !tmpX->empty())
	    tmpOther->push_back(unop::instance(unop::X,
					       multop::instance(mo->op(),
								tmpX)));
	  else
	    delete tmpX;

	  if (!tmpF->empty())
	    tmpOther->push_back(unop::instance(unop::F,
					       multop::instance(mo->op(),
								tmpF)));
	  else
	    delete tmpF;

	  if (!tmpG->empty())
	    tmpOther->push_back(unop::instance(unop::G,
					       multop::instance(mo->op(),
								tmpG)));
	  else
	    delete tmpG;

	  if (!tmpU->empty())
	    tmpOther->push_back(multop::instance(mo->op(), tmpU));
	  else
	    delete tmpU;


	  if (!tmpR->empty())
	    tmpOther->push_back(multop::instance(mo->op(), tmpR));
	  else
	    delete tmpR;

	  if (!tmpGF->empty())
	    {
	      formula* ftmp
		= unop::instance(unop::G,
				 unop::instance(unop::F,
						multop::instance(mo->op(),
								 tmpGF)));
	      tmpOther->push_back(ftmp);
	    }
	  else
	    delete tmpGF;


	  if (!tmpFG->empty())
	    {
	      formula* ftmp = 0;
	      if (mo->op() == multop::And)
		ftmp
		  = unop::instance(unop::F,
				   unop::instance(unop::G,
						  multop::instance(mo->op(),
								   tmpFG)));
	      else
		ftmp
		  = unop::instance(unop::F,
				   multop::instance(mo->op(), tmpFG));
	      tmpOther->push_back(ftmp);
	    }
	  else
	    delete tmpFG;


	  result_ = multop::instance(op, tmpOther);

	  return;
	}

      protected:
	formula* result_;
      };
    }

    formula*
    basic_reduce(const formula* f)
    {
      basic_reduce_visitor v;
      const_cast<formula*>(f)->accept(v);
      return v.result();
    }

  }
}
