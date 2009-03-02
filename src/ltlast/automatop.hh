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

/// \file ltlast/automatop.hh
/// \brief ELTL automaton operators
#ifndef SPOT_LTLAST_AUTOMATOP_HH
# define SPOT_LTLAST_AUTOMATOP_HH

# include <vector>
# include <map>
# include "nfa.hh"
# include "refformula.hh"

namespace spot
{
  namespace ltl
  {
    /// \brief Automaton operators.
    /// \ingroup eltl_ast
    ///
    class automatop : public ref_formula
    {
    public:
      /// List of formulae.
      typedef std::vector<formula*> vec;

      /// \brief Build a spot::ltl::automatop with many children.
      ///
      /// This vector is acquired by the spot::ltl::automatop class,
      /// the caller should allocate it with \c new, but not use it
      /// (especially not destroy it) after it has been passed to
      /// spot::ltl::automatop.
      static automatop*
      instance(const nfa::ptr nfa, vec* v);

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

      /// Get the NFA of this operator.
      const nfa::ptr nfa() const;

      bool negated_;
    protected:
      typedef std::pair<nfa::ptr, vec*> pair;
      /// Comparison functor used internally by ltl::automatop.
      struct paircmp
      {
	bool
	operator () (const pair& p1, const pair& p2) const
	{
	  if (p1.first != p2.first)
	    return p1.first < p2.first;
	  return *p1.second < *p2.second;
	}
      };
      typedef std::map<pair, formula*, paircmp> map;
      static map instances;

      automatop(const nfa::ptr nfa, vec* v);
      virtual ~automatop();

    private:
      nfa::ptr nfa_;
      vec* children_;
    };
  }
}

#endif // SPOT_LTLAST_AUTOMATOP_HH
