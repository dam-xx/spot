// Copyright (C) 2008 Laboratoire d'Informatique de Paris 6 (LIP6),
// d�partement Syst�mes R�partis Coop�ratifs (SRC), Universit� Pierre
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

#ifndef SPOT_ELTLVISIT_POSTFIX_HH
# define SPOT_ELTLVISIT_POSTFIX_HH

#include "eltlast/formula.hh"

namespace spot
{
  namespace eltl
  {
    /// \brief Apply an algorithm on each node of an AST,
    /// during a postfix traversal.
    /// \ingroup eltl_visitor
    ///
    /// Override one or more of the postifix_visitor::doit methods
    /// with the algorithm to apply.
    class postfix_visitor : public visitor
    {
    public:
      postfix_visitor();
      virtual ~postfix_visitor();

      void visit(atomic_prop* ap);
      void visit(unop* uo);
      void visit(binop* bo);
      void visit(multop* mo);
      void visit(constant* c);
      void visit(automatop* ao);

      virtual void doit(atomic_prop* ap);
      virtual void doit(unop* uo);
      virtual void doit(binop* bo);
      virtual void doit(multop* mo);
      virtual void doit(constant* c);
      virtual void doit(automatop* ao);
      virtual void doit_default(formula* f);
    };
  }
}

#endif // SPOT_ELTLVISIT_POSTFIX_HH