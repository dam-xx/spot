// Copyright (C) 2008 Laboratoire d'Informatique de Paris 6 (LIP6),
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

#include "automatop.hh"

namespace spot
{
  namespace eltl
  {
    automatop::automatop(vec* v)
      : nfa_(), children_(v)
    {
      dump_ = "automatop(";
      dump_ += (*v)[0]->dump();
      for (unsigned n = 1; n < v->size(); ++n)
	dump_ += ", " + (*v)[n]->dump();
      dump_ += ")";
      set_key_();
    }

    automatop::~automatop()
    {
      delete children_;
    }

    void
    automatop::accept(visitor& v)
    {
      v.visit(this);
    }

    void
    automatop::accept(const_visitor& v) const
    {
      v.visit(this);
    }

    automatop*
    automatop::instance(nfa::ptr nfa, formula* autop)
    {
      vec* v = new vec;
      v->push_back(autop);
      automatop* res = instance(v);
      res->nfa_ = nfa;
      return res;
    }

    automatop*
    automatop::instance(formula* first, formula* second)
    {
      vec* v = new vec;
      v->push_back(first);
      v->push_back(second);
      return instance(v);
    }

    automatop*
    automatop::instance(vec* v)
    {
      // Inline children of same kind.
      {
	vec inlined;
	vec::iterator i = v->begin();
	while (i != v->end())
	{
	  if (automatop* p = dynamic_cast<automatop*>(*i))
	  {
	    unsigned ps = p->size();
	    for (unsigned n = 0; n < ps; ++n)
	      inlined.push_back(p->nth(n));
	    formula::unref(*i);
	    i = v->erase(i);
	  }
	  else
	    ++i;
	}
	v->insert(v->end(), inlined.begin(), inlined.end());
      }

      automatop* res = new automatop(v);
      return static_cast<automatop*>(res->ref());
    }

    unsigned
    automatop::size() const
    {
      return children_->size();
    }

    const formula*
    automatop::nth(unsigned n) const
    {
      return (*children_)[n];
    }

  formula*
    automatop::nth(unsigned n)
    {
      return (*children_)[n];
    }
  }
}
