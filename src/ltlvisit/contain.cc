// Copyright (C) 2006 Laboratoire d'Informatique de Paris 6 (LIP6),
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

#include "contain.hh"
#include "destroy.hh"
#include "clone.hh"
#include "tunabbrev.hh"
#include "ltlast/unop.hh"
#include "ltlast/binop.hh"
#include "ltlast/multop.hh"
#include "ltlast/constant.hh"
#include "tgba/tgbaproduct.hh"
#include "tgbaalgos/gtec/gtec.hh"
#include "tgbaalgos/save.hh"
#include "tostring.hh"
#include <iostream>
namespace spot
{
  namespace ltl
  {

    language_containment_checker::language_containment_checker
      (bdd_dict* dict, bool exprop, bool symb_merge,
       bool branching_postponement, bool fair_loop_approx)
      : dict_(dict), exprop_(exprop), symb_merge_(symb_merge),
      branching_postponement_(branching_postponement),
      fair_loop_approx_(fair_loop_approx)
    {
    }

    language_containment_checker::~language_containment_checker()
    {
      while (!translated_.empty())
	{
	  trans_map::iterator i = translated_.begin();
	  delete i->second.translation;
	  const formula* f = i->first;
	  translated_.erase(i);
	  destroy(f);
	}
    }

    // Check whether L(l) is a subset of L(g).
    bool
    language_containment_checker::contained(const formula* l, const formula* g)
    {
      const record_* rl = register_formula_(l);
      const formula* ng = unop::instance(unop::Not, clone(g));
      const record_* rng = register_formula_(ng);
      destroy(ng);
      bool res = rl->incompatible.find(rng) != rl->incompatible.end();
      return res;
    }

    // Check whether L(!l) is a subset of L(g).
    bool
    language_containment_checker::neg_contained(const formula* l,
						const formula* g)
    {
      const formula* nl = unop::instance(unop::Not, clone(l));
      const record_* rnl = register_formula_(nl);
      const formula* ng = unop::instance(unop::Not, clone(g));
      const record_* rng = register_formula_(ng);
      destroy(nl);
      destroy(ng);
      bool res = rnl->incompatible.find(rng) != rnl->incompatible.end();
      return res;
    }

    // Check whether L(l) is a subset of L(!g).
    bool
    language_containment_checker::contained_neg(const formula* l,
						const formula* g)
    {
      const record_* rl = register_formula_(l);
      const record_* rg = register_formula_(g);
      bool res = rl->incompatible.find(rg) != rl->incompatible.end();
      return res;
    }

    // Check whether L(l) = L(g).
    bool
    language_containment_checker::equal(const formula* l, const formula* g)
    {
      return contained(l,g) && contained(g,l);
    }

    const language_containment_checker::record_*
    language_containment_checker::register_formula_(const formula* f)
    {
      trans_map::iterator i = translated_.find(f);
      if (i != translated_.end())
	return &i->second;

      const tgba_explicit* e = ltl_to_tgba_fm(f, dict_, exprop_, symb_merge_,
					      branching_postponement_,
					      fair_loop_approx_);
      record_& r = translated_[clone(f)];
      r.translation = e;

      // Check the emptiness of the product of this formula with any
      // other registered formula.
      for (i = translated_.begin(); i != translated_.end(); ++i)
	{
	  if (f == i->first)
	    continue;
	  const tgba* p = new tgba_product(e, i->second.translation);
	  emptiness_check* ec = couvreur99(p);
	  emptiness_check_result* ecr = ec->check();
	  if (!ecr)
	    {
	      r.incompatible.insert(&i->second);
	      i->second.incompatible.insert(&r);
	    }
	  else
	    delete ecr;
	  delete ec;
	  delete p;
	}
      return &r;
    }


    namespace {
      struct reduce_tau03_visitor : public clone_visitor {
	bool stronger;
	language_containment_checker* lcc;

	reduce_tau03_visitor(bool stronger,
			     language_containment_checker* lcc)
	  : stronger(stronger), lcc(lcc)
	{
	}

	void
	visit(unop* uo)
	{
	  formula* a = recurse(uo->child());
	  switch (uo->op())
	    {
	    case unop::X:
	      // if X(a) = a, then keep only a !
	      if (stronger && lcc->equal(a, uo))
		{
		  result_ = a;
		  break;
		}
	    default:
	      result_ = unop::instance(uo->op(), a);
	    }
	}

	void
	visit(binop* bo)
	{
	  formula* a = recurse(bo->first());
	  formula* b = recurse(bo->second());
	  switch (bo->op())
	    {
	    case binop::U:
	      // if (a U b) = b, then keep b !
	      if (stronger && lcc->equal(bo, b))
		{
		  destroy(a);
		  result_ = b;
		}
	      // if a => b,  then a U b = b.
	      else if ((!stronger) && lcc->contained(a, b))
		{
		  destroy(a);
		  result_ = b;
		}
	      // if !a => b, then  a U b = Fb
	      else if (lcc->neg_contained(a, b))
		{
		  destroy(a);
		  result_ = unop::instance(unop::F, b);
		}
	      else
		{
		  result_ = binop::instance(binop::U, a, b);
		}
	      break;
	    case binop::R:
	      // if (a R b) = b, then keep b !
	      if (stronger && lcc->equal(bo, b))
		{
		  destroy(a);
		  result_ = b;
		}
	      // if b => a,  then a R b = b.
	      else if ((!stronger) && lcc->contained(b, a))
		{
		  destroy(a);
		  result_ = b;
		}
	      // if a => !b, then  a R b = Gb
	      else if (lcc->contained_neg(a, b))
		{
		  destroy(a);
		  result_ = unop::instance(unop::G, b);
		}
	      else
		{
		  result_ = binop::instance(binop::R, a, b);
		}
	      break;
	    default:
	      result_ = binop::instance(bo->op(), a, b);
	    }
	}

	void
	visit(multop* mo)
	{
	  multop::vec* res = new multop::vec;
	  unsigned mos = mo->size();
	  for (unsigned i = 0; i < mos; ++i)
	    res->push_back(recurse(mo->nth(i)));
	  result_ = 0;
	  bool changed = false;

	  switch (mo->op())
	    {
	    case multop::Or:
	      for (unsigned i = 0; i < mos; ++i)
		{
		  if (!(*res)[i])
		    continue;
		  for (unsigned j = i + 1; j < mos; ++j)
		    {
		      if (!(*res)[j])
			continue;
		      // if !i => j, then i|j = true
		      if (lcc->neg_contained((*res)[i], (*res)[j]))
			{
			  result_ = constant::true_instance();
			  goto constant_;
			}
		      // if i => j, then i|j = j
		      else if (lcc->contained((*res)[i], (*res)[j]))
			{
			  destroy((*res)[i]);
			  (*res)[i] = 0;
			  changed = true;
			  break;
			}
		      // if j => i, then i|j = i
		      else if (lcc->contained((*res)[i], (*res)[j]))
			{
			  destroy((*res)[j]);
			  (*res)[j] = 0;
			  changed = true;
			}
		    }
		}
	      break;
	    case multop::And:
	      for (unsigned i = 0; i < mos; ++i)
		{
		  if (!(*res)[i])
		    continue;
		  for (unsigned j = i + 1; j < mos; ++j)
		    {
		      if (!(*res)[j])
			continue;
		      // if i => !j, then i&j = false
		      if (lcc->contained_neg((*res)[i], (*res)[j]))
			{
			  result_ = constant::false_instance();
			  goto constant_;
			}
		      // if i => j, then i&j = i
		      else if (lcc->contained((*res)[i], (*res)[j]))
			{
			  destroy((*res)[j]);
			  (*res)[j] = 0;
			  changed = true;
			}
		      // if j => i, then i&j = j
		      else if (lcc->contained((*res)[i], (*res)[j]))
			{
			  destroy((*res)[i]);
			  (*res)[i] = 0;
			  changed = true;
			  break;
			}
		    }
		}
	      break;
	    }
	  if (changed)
	    {
	      multop::vec* nres = new multop::vec;
	      for (unsigned i = 0; i < mos; ++i)
		if ((*res)[i])
		  nres->push_back((*res)[i]);
	      delete res;
	      res = nres;
	    }
	  result_ = multop::instance(mo->op(), res);
	  return;
	constant_:
	  for (unsigned i = 0; i < mos; ++i)
	    if ((*res)[i])
	      destroy((*res)[i]);
	}

	formula*
	recurse(formula* f)
	{
	  reduce_tau03_visitor v(stronger, lcc);
	  const_cast<formula*>(f)->accept(v);
	  return v.result();
	}
      };
    }

    formula*
    reduce_tau03(const formula* f, bool stronger)
    {
      bdd_dict b;
      reduce_tau03_visitor v(stronger,
			     new language_containment_checker(&b,
							      true, true,
							      false, false));
      // reduce_tau03_visitor does not handle Xor, Implies, and Equiv.
      f = unabbreviate_ltl(f);
      const_cast<formula*>(f)->accept(v);
      destroy(f);
      delete v.lcc;
      return v.result();
    }
  }
}
