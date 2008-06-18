// Copyright (C) 2003, 2004, 2005, 2008 Laboratoire d'Informatique de
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

/// \file internal/multop.hxx
/// \brief Generic multi-operand operators implementation
#ifndef SPOT_INTERNAL_MULTOP_HXX
# define SPOT_INTERNAL_MULTOP_HXX

#include "multop.hh"

namespace spot
{
  namespace internal
  {
    template<typename T>
    multop<T>::multop(type op, vec* v)
      : op_(op), children_(v)
    {
      this->dump_ = "multop(";
      this->dump_ += op_name();
      unsigned max = v->size();
      for (unsigned n = 0; n < max; ++n)
	{
	  this->dump_ += ", " + (*v)[n]->dump();
	}
      this->dump_ += ")";
      this->set_key_();
    }

    template<typename T>
    multop<T>::~multop()
    {
      // Get this instance out of the instance map.
      pair p(op(), children_);
      typename map::iterator i = instances.find(p);
      assert (i != instances.end());
      instances.erase(i);

      delete children_;
    }

    template<typename T>
    void
    multop<T>::accept(visitor& v)
    {
      v.visit(this);
    }

    template<typename T>
    void
    multop<T>::accept(const_visitor& v) const
    {
      v.visit(this);
    }

    template<typename T>
    unsigned
    multop<T>::size() const
    {
      return children_->size();
    }

    template<typename T>
    const formula<T>*
    multop<T>::nth(unsigned n) const
    {
      return (*children_)[n];
    }

    template<typename T>
    formula<T>*
    multop<T>::nth(unsigned n)
    {
      return (*children_)[n];
    }

    template<typename T>
    typename multop<T>::type
    multop<T>::op() const
    {
      return op_;
    }

    template<typename T>
    const char*
    multop<T>::op_name() const
    {
      switch (op_)
	{
	case And:
	  return "And";
	case Or:
	  return "Or";
	}
      // Unreachable code.
      assert(0);
      return 0;
    }

    template<typename T>
    typename multop<T>::map multop<T>::instances;

    template<typename T>
    formula<T>*
    multop<T>::instance(type op, vec* v)
    {
      pair p(op, v);

      // Inline children of same kind.
      {
	vec inlined;
	typename vec::iterator i = v->begin();
	while (i != v->end())
	  {
	    multop<T>* p = dynamic_cast<multop<T>*>(*i);
	    if (p && p->op() == op)
	      {
		unsigned ps = p->size();
		for (unsigned n = 0; n < ps; ++n)
		  inlined.push_back(p->nth(n));
		// That sub-formula is now useless, drop it.
		// Note that we use unref(), not destroy(), because we've
		// adopted its children and don't want to destroy these.
		formula<T>::unref(*i);
		i = v->erase(i);
	      }
	    else
	      {
		++i;
	      }
	  }
	v->insert(v->end(), inlined.begin(), inlined.end());
      }

      std::sort(v->begin(), v->end(), formula_ptr_less_than());

      // Remove duplicates.  We can't use std::unique(), because we
      // must destroy() any formula we drop.
      {
	formula<T>* last = 0;
	typename vec::iterator i = v->begin();
	while (i != v->end())
	  {
	    if (*i == last)
	      {
		destroy(*i);
		i = v->erase(i);
	      }
	    else
	      {
		last = *i++;
	      }
	  }
	}

      typename vec::size_type s = v->size();
      if (s == 0)
	{
	  delete v;
	  switch (op)
	    {
	    case And:
	      return constant<T>::true_instance();
	    case Or:
	      return constant<T>::false_instance();
	    }
	  /* Unreachable code.  */
	  assert(0);
	}
      else if (s == 1)
	{
	  formula<T>* res = (*v)[0];
	  delete v;
	  return res;
	}

      typename map::iterator i = instances.find(p);
      if (i != instances.end())
	{
	  delete v;
	  return static_cast<multop<T>*>(i->second->ref());
	}
      multop<T>* ap = new multop<T>(op, v);
      instances[p] = ap;
      return static_cast<multop<T>*>(ap->ref());

    }

    template<typename T>
    formula<T>*
    multop<T>::instance(type op, formula<T>* first, formula<T>* second)
    {
      vec* v = new vec;
      v->push_back(first);
      v->push_back(second);
      return instance(op, v);
    }

    template<typename T>
    unsigned
    multop<T>::instance_count()
    {
      return instances.size();
    }

    template<typename T>
    void
    multop<T>::destroy(formula<T>* c)
    {
      if (unop<T>* uo = dynamic_cast<unop<T>*>(c))
      {
	destroy(uo->child());
	formula<T>::unref(uo);
      }
      else if (binop<T>* bo = dynamic_cast<binop<T>*>(c))
      {
	destroy(bo->first());
	destroy(bo->second());
	formula<T>::unref(bo);
      }
      else if (multop<T>* mo = dynamic_cast<multop<T>*>(c))
      {
	unsigned s = mo->size();
	for (unsigned i = 0; i < s; ++i)
	  destroy(mo->nth(i));
	formula<T>::unref(mo);
      }
      else // constant<T>* || atomic_prop<T>*
	formula<T>::unref(c);
    }
  }
}

#endif // SPOT_INTERNAL_MULTOP_HXX
