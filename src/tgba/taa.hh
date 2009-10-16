// Copyright (C) 2009 Laboratoire d'Informatique de Paris
// 6 (LIP6), département Systèmes Répartis Coopératifs (SRC),
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

#ifndef SPOT_TGBA_TAA_HH
# define SPOT_TGBA_TAA_HH

#include <set>
#include <vector>
#include "misc/hash.hh"
#include "ltlast/formula.hh"
#include "bdddict.hh"
#include "tgba.hh"

namespace spot
{
  /// \brief A Transition-based Alternating Automaton (TAA).
  class taa : public tgba
  {
  public:
    taa(bdd_dict* dict);

    struct transition;
    typedef std::list<transition*> state;
    typedef std::set<state*> state_set;

    /// Explicit transitions.
    struct transition
    {
      bdd condition;
      bdd acceptance_conditions;
      const state_set* dst;
    };

    void set_init_state(const std::string& state);

    transition*
    create_transition(const std::string& src,
		      const std::vector<std::string>& dst);
    transition*
    create_transition(const std::string& src, const std::string& dst);

    void add_condition(transition* t, const ltl::formula* f);
    void add_acceptance_condition(transition* t, const ltl::formula* f);

    /// TGBA interface.
    virtual ~taa();
    virtual spot::state* get_init_state() const;
    virtual tgba_succ_iterator*
    succ_iter(const spot::state* local_state,
	      const spot::state* global_state = 0,
	      const tgba* global_automaton = 0) const;
    virtual bdd_dict* get_dict() const;

    /// \brief Format the state as a string for printing.
    ///
    /// If state is a spot::state_set of only one element, then the
    /// string corresponding to state->get_state() is returned.
    ///
    /// Otherwise a string composed of each string corresponding to
    /// each state->get_state() in the spot::state_set is returned,
    /// e.g. like {string_1,...,string_n}.
    virtual std::string format_state(const spot::state* state) const;

    virtual bdd all_acceptance_conditions() const;
    virtual bdd neg_acceptance_conditions() const;

  protected:
    virtual bdd compute_support_conditions(const spot::state* state) const;
    virtual bdd compute_support_variables(const spot::state* state) const;

    typedef Sgi::hash_map<
      const std::string, taa::state*, string_hash
      > ns_map;

    typedef Sgi::hash_map<
      const taa::state*, std::string, ptr_hash<taa::state>
      > sn_map;

    typedef std::vector<taa::state_set*> ss_vec;

    ns_map name_state_map_;
    sn_map state_name_map_;
    bdd_dict* dict_;
    mutable bdd all_acceptance_conditions_;
    mutable bool all_acceptance_conditions_computed_;
    bdd neg_acceptance_conditions_;
    taa::state* init_;
    ss_vec state_set_vec_;

  private:
    // Disallow copy.
    taa(const taa& other);
    taa& operator=(const taa& other);

    /// \brief Return the taa::state for \a name, creating it if it
    /// does not exist.  The first state added is the initial state
    /// which can be overridden with set_init_state.
    taa::state* add_state(const std::string& name);

    /// \brief Return the taa::state_set for \a names.
    taa::state_set* add_state_set(const std::vector<std::string>& names);
  };

  /// Set of states deriving from spot::state.
  class state_set : public spot::state
  {
  public:
    /// The taa::state_set has been allocated with \c new.  It is the
    /// responsability of the state_set to \c delete it when no longer
    /// needed (cf. dtor).
    state_set(const taa::state_set* s)
       : s_(s)
    {
    }

    virtual int compare(const spot::state*) const;
    virtual size_t hash() const;
    virtual state_set* clone() const;

    virtual ~state_set()
    {
      delete s_;
    }

    const taa::state_set* get_state() const;
  private:
    const taa::state_set* s_;
  };

  class taa_succ_iterator : public tgba_succ_iterator
  {
  public:
    taa_succ_iterator(const taa::state_set* s, bdd all_acc);

    virtual ~taa_succ_iterator()
    {
    }

    virtual void first();
    virtual void next();
    virtual bool done() const;

    virtual state_set* current_state() const;
    virtual bdd current_condition() const;
    virtual bdd current_acceptance_conditions() const;

  private:
    typedef taa::state::const_iterator iterator;

    std::vector<std::pair<iterator, iterator> > bounds_;
    std::vector<iterator> its_;
    bdd all_acceptance_conditions_;
    mutable bool empty_;
  };
}

#endif // SPOT_TGBA_TAA_HH
