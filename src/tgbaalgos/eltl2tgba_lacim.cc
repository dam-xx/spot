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

#include "eltlast/formula.hh"
#include "eltlvisit/lunabbrev.hh"
#include "eltlvisit/nenoform.hh"
#include "eltlvisit/destroy.hh"
#include "tgba/tgbabddconcretefactory.hh"
#include <cassert>

#include "eltl2tgba_lacim.hh"

namespace spot
{
  namespace
  {
    using namespace eltl;

    /// \brief Recursively translate a formula into a BDD.
    class eltl_trad_visitor: public const_visitor
    {
    public:
      eltl_trad_visitor(tgba_bdd_concrete_factory& fact, bool root = false)
	: fact_(fact), root_(root)
      {
      }

      virtual
      ~eltl_trad_visitor()
      {
      }

      bdd
      result()
      {
	return res_;
      }

      void
      visit(const atomic_prop* node)
      {
	res_ = bdd_ithvar(fact_.create_atomic_prop(node));
      }

      void
      visit(const constant* node)
      {
	switch (node->val())
	  {
	  case constant::True:
	    res_ = bddtrue;
	    return;
	  case constant::False:
	    res_ = bddfalse;
	    return;
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(const unop* node)
      {
	switch (node->op())
	  {
	  case unop::Not:
	    {
	      res_ = bdd_not(recurse(node->child()));
	      return;
	    }
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(const binop* node)
      {
	bdd f1 = recurse(node->first());
	bdd f2 = recurse(node->second());

	switch (node->op())
	  {
	  case binop::Xor:
	    res_ = bdd_apply(f1, f2, bddop_xor);
	    return;
	  case binop::Implies:
	    res_ = bdd_apply(f1, f2, bddop_imp);
	    return;
	  case binop::Equiv:
	    res_ = bdd_apply(f1, f2, bddop_biimp);
	    return;
	  }
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(const multop* node)
      {
	int op = -1;
	bool root = false;
	switch (node->op())
	  {
	  case multop::And:
	    op = bddop_and;
	    res_ = bddtrue;
	    // When the root formula is a conjunction it's ok to
	    // consider all children as root formulae.  This allows the
	    // root-G trick to save many more variable.  (See the
	    // translation of G.)
	    root = root_;
	    break;
	  case multop::Or:
	    op = bddop_or;
	    res_ = bddfalse;
	    break;
	  }
	assert(op != -1);
	unsigned s = node->size();
	for (unsigned n = 0; n < s; ++n)
	  {
	    res_ = bdd_apply(res_, recurse(node->nth(n), root), op);
	  }
      }

      void
      visit (const automatop* node)
      {
	// FIXME.
	(void) node;
      }

      bdd
      recurse(const formula* f, bool root = false)
      {
	eltl_trad_visitor v(fact_, root);
	f->accept(v);
	return v.result();
      }

    private:
      bdd res_;
      tgba_bdd_concrete_factory& fact_;
      bool root_;
    };
  } // anonymous

  tgba_bdd_concrete*
  eltl_to_tgba_lacim(const eltl::formula* f, bdd_dict* dict)
  {
    // Normalize the formula.  We want all the negations on
    // the atomic propositions.  We also suppress logic
    // abbreviations such as <=>, =>, or XOR, since they
    // would involve negations at the BDD level.
    const eltl::formula* f1 = eltl::unabbreviate_logic(f);
    const eltl::formula* f2 = eltl::negative_normal_form(f1);
    eltl::destroy(f1);

    // Traverse the formula and draft the automaton in a factory.
    tgba_bdd_concrete_factory fact(dict);
    eltl_trad_visitor v(fact, true);
    f2->accept(v);
    eltl::destroy(f2);
    fact.finish();

    // Finally setup the resulting automaton.
    return new tgba_bdd_concrete(fact, v.result());
  }
}
