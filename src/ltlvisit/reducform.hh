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

#ifndef SPOT_LTLVISIT_REDUCFORM_HH
# define SPOT_LTLVISIT_REDUCFORM_HH

#include "ltlast/formula.hh"
#include "ltlast/visitor.hh"

// For debog
#include "ltlvisit/dump.hh"

namespace spot
{
  namespace ltl
  {
    formula* reduce(const formula* f);

    /* Basic rewriting */
    formula* basic_reduce_form(const formula* f);
    
    /* formula rewriting using univerality, eventuality, 
       implies and basic_reduce_form */
    formula* reduce_form(const formula* f);

    /* detect easy case of implies */
    bool inf_form(const formula* f1, const formula* f2); 
    /* true if f1 < f2, false otherwise */
    bool infneg_form(const formula* f1, const formula* f2, int n); 
    /* true if !f1 < f2, false otherwise */
    
    /* detect if a formula is of class eventuality or universality */
    bool is_eventual(const formula* f);
    bool is_universal(const formula* f);

    /* detect if a formula is of form GF or FG */
    bool is_GF(const formula* f);
    bool is_FG(const formula* f);
      
    /* To know the first node of a formula */
    
    class node_type_form_visitor : public const_visitor
    {
    public:
      enum type { Atom, Const, Unop, Binop, Multop };
      node_type_form_visitor();
      virtual ~node_type_form_visitor(){};
      type result() const;
      void visit(const atomic_prop* ap);
      void visit(const constant* c);
      void visit(const unop* uo);
      void visit(const binop* bo);
      void visit(const multop* mo);
    protected:
      type result_;
    };
    
    node_type_form_visitor::type node_type(const formula* f);

    /* Obsolete */
    int nb_term_multop(const formula* f);
    formula* reduce_inf_form(const formula* f); /* Obsolete */
    int form_length(const formula* f); /* For test */
    
  }
}

#endif //  SPOT_LTLVISIT_REDUCFORM_HH
