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

/// \file eltlast/automatop.hh
/// \brief ELTL automaton operators
#ifndef SPOT_ELTLAST_AUTOMATOP_HH
# define SPOT_ELTLAST_AUTOMATOP_HH

# include "multop.hh"
# include "refformula.hh"
# include "nfa.hh"

namespace spot
{
  namespace eltl
  {

    /// \brief Automaton operators.
    /// \ingroup eltl_ast
    ///
    class automatop : public ref_formula
    {
    public:
      /// List of formulae.
      typedef std::vector<formula*> vec;

      /// \brief Build a spot::eltl::automatop with automaton \c nfa
      /// and children of \c autop.
      static automatop* instance(nfa::ptr nfa, formula* autop);

      /// \brief Build a spot::eltl::automatop with two children.
      ///
      /// If one of the children itself is a spot::eltl::automatop,
      /// it will be merged. This allows incremental building of
      /// n-ary eltl::automatop.
      static automatop* instance(formula* first, formula* second);

      /// \brief Build a spot::eltl::automatop with many children.
      static automatop* instance(vec* v);

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      /// Get the number of argument.
      unsigned size() const;
      /// \brief Get the nth argument.
      ///
      /// Starting with \a n = 0.
      const formula* nth(unsigned n) const;
      /// \brief Get the nth argument.
      ///
      /// Starting with \a n = 0.
      formula* nth(unsigned n);

    protected:
      automatop(vec* v);
      virtual ~automatop();

    private:
      nfa::ptr nfa_;
      vec* children_;
    };
  }
}

#endif // SPOT_ELTLAST_AUTOMATOP_HH
