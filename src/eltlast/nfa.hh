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

/// \file eltlast/nfa.hh
/// \brief NFA interface
#ifndef SPOT_ELTLAST_NFA_HH
# define SPOT_ELTLAST_NFA_HH

# include "misc/hash.hh"
# include <boost/shared_ptr.hpp>
# include <list>
# include <set>

namespace spot
{
  namespace eltl
  {
    /// Forward declaration. See below.
    class succ_iterator;

    /// \brief A Nondeterministic Finite Automaton used in ELTL
    /// automata operators.
    /// \ingroup eltl_essential
    class nfa
    {
    public:
      struct				transition;
      typedef std::list<transition*>	state;
      /// Iterator over the successors of a state.
      typedef succ_iterator		iterator;
      typedef boost::shared_ptr<nfa>	ptr;

      /// Explicit transitions.
      struct transition
      {
	int cost;
      	const state* dest;
      };

      nfa();
      ~nfa();

      void
      add_transition(const std::string& s, const std::string& d, int c);

      void
      set_init_state(const std::string& state);
      const state*
      get_init_state();

      void
      set_final(const std::string& state);
      bool
      is_final(const std::string& state);

      /// \brief Get the `arity' i.e. max t.cost, for each transition t.
      int
      arity();

      /// \brief Return an iterator on the first succesor (if any) of \a state.
      ///
      /// The usual way to do this with a \c for lopp.
      /// \code
      ///    for (nfa::iterator i = a.begin(state); i != a.end(state); ++i)
      ///      ...
      /// \endcode
      iterator
      begin(const state* state) const;

      /// \brief Return an iterator just past the last succesor of \a state.
      iterator
      end(const state* state) const;

      std::string
      format_state(const state* state) const;

    private:
      state*
      add_state(const std::string& name);

      typedef Sgi::hash_map<const std::string, state*, string_hash> ns_map;
      typedef Sgi::hash_map<const state*, std::string, ptr_hash<state> > sn_map;

      ns_map ns_;
      sn_map sn_;
      state* init_;

      int arity_;
      std::set<std::string> finals_;

      /// Explicitly disllow use of implicity generated member functions
      /// we don't want.
      nfa(const nfa& other);
      nfa& operator=(const nfa& other);
    };

    class succ_iterator
    {
    public:
      succ_iterator(const nfa::state::const_iterator s)
	: i_(s)
      {
      }

      void
      operator++()
      {
	++i_;
      }

      bool
      operator!=(const succ_iterator& rhs) const
      {
	return i_ != rhs.i_;
      }

      const nfa::transition* operator*() const
      {
	return *i_;
      }

    private:
      nfa::state::const_iterator i_;
    };

  }
}

#endif // SPOT_ELTLAST_NFA_HH_
