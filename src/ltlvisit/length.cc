// Copyright (C) 2010 Laboratoire de Recherche et Développement de
// l'Epita (LRDE).
// Copyright (C) 2004, 2005  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#include "length.hh"
#include "ltlvisit/postfix.hh"
#include "ltlast/multop.hh"

namespace spot
{
  namespace ltl
  {
    namespace
    {
      class length_visitor: public postfix_visitor
      {
      public:
	length_visitor()
	  : result_(0)
	{
	}

	int
	result() const
	{
	  return result_;
	}

	virtual void
	visit(multop* mo)
	{
	  unsigned s = mo->size();
	  for (unsigned i = 0; i < s; ++i)
	    mo->nth(i)->accept(*this);
	  // "a & b & c" should count for 5, even though it is
	  // stored as And(a,b,c).
	  mo += s - 1;
	}

	virtual void
	doit_default(formula*)
	{
	  ++result_;
	}

      protected:
	int result_; // size of the formula
      };
    }

    int
    length(const formula* f)
    {
      length_visitor v;
      const_cast<formula*>(f)->accept(v);
      return v.result();
    }

  }
}
