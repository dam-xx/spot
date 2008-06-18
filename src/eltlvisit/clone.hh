// Copyright (C) 2008  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#ifndef SPOT_ELTLVISIT_CLONE_HH
# define SPOT_ELTLVISIT_CLONE_HH

#include "eltlast/formula.hh"

namespace spot
{
  namespace eltl
  {
    /// \brief Clone a formula.
    /// \ingroup eltl_visitor
    ///
    /// This visitor is public, because it's convenient to derive from
    /// it and override part of its methods.  But if you just want the
    /// functionality, consider using spot::eltl::clone instead.
    class clone_visitor : public visitor
    {
    public:
      clone_visitor();
      virtual ~clone_visitor();

      formula* result() const;

      void visit(atomic_prop* ap);
      void visit(unop* uo);
      void visit(binop* bo);
      void visit(multop* mo);
      void visit(constant* c);
      void visit(automatop* ao);

      virtual formula* recurse(formula* f);

    protected:
      formula* result_;
    };

    /// \brief Clone a formula.
    /// \ingroup eltl_essential
    formula* clone(const formula* f);
  }
}

#endif // SPOT_ELTLVISIT_LUNABBREV_HH
