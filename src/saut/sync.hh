// Copyright (C) 2006 Laboratoire d'Informatique de
// Paris 6 (LIP6), département Systèmes Répartis Coopératifs (SRC),
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

#ifndef SPOT_SYNC_HH
# define SPOT_SYNC_HH

#include "tgba/tgba.hh"
#include "misc/hash.hh"
#include <list>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "saut.hh"

namespace spot
{
  class sync_state;
  class sync_state_heap;
  class sync_transition;

  class sync : public tgba
  {
  public:
    typedef std::vector<const saut::node*> vnodes;
    typedef std::list<const saut*> saut_list;
    typedef std::vector<const saut*> autvec;
    typedef std::vector<size_t> autkvec;
    typedef std::list<const saut::action_name*> action_list;
    struct action_vect
    {
      typedef std::vector<const saut::action*> actvect;
      actvect v;
      enum Fairness { None, Weak, Strong } f;
      int acc;
      action_vect(const actvect& v) : v(v), f(None), acc(-1) {};
      const saut::action*& operator[](unsigned n)       { return v[n]; }
      const saut::action*  operator[](unsigned n) const { return v[n]; }
    };
    typedef Sgi::hash_multimap<size_t, action_vect> action_map;
    typedef std::list<const action_vect*> action_vect_list;
    typedef std::map<const saut::action*, action_vect_list> action_back_map;
    typedef std::vector<action_back_map> action_back_vector;
    typedef std::map<const sync::action_vect*, int> action_weak_map;
    typedef std::map<const sync::action_vect*,
		     std::pair<int, int> > action_strong_map;
  private:
    autvec auts;
    autkvec autsk;
    unsigned autssize;
    sync_state_heap* heap;
    bdd_dict* dict;
    action_map actions;
    action_back_vector actions_back;
    bool stubborn;
    bdd aphi;
    action_weak_map actweak;
    action_strong_map actstrong;
    bdd allacc;
    bdd allweak;
    bdd allacc_neg;
  public:
    sync(saut_list& sautlist, bool stubborn = false);
    virtual ~sync();
    bool known_action(unsigned aut_num, const saut::action_name& act) const;

    unsigned size() const { return auts.size(); }
    const saut* aut(unsigned n) const { return auts[n]; }

    action_vect* declare_rule(action_list& l);
    void set_stubborn(bool val = true);
    bool get_stubborn() const;
    void set_aphi(bdd aphi);
    std::string set_aphi(ltl::formula* f);
    bdd get_aphi() const;

    bool known_proposition(const ltl::atomic_prop* ap) const;

    void
    set_fairness(action_vect* v, action_vect::Fairness f);

    virtual state* get_init_state() const;
    virtual bdd_dict* get_dict() const;
    std::string format_state(const vnodes& nodes) const;
    virtual std::string format_state(const state* s) const;
    virtual bdd all_acceptance_conditions() const;
    virtual bdd neg_acceptance_conditions() const;
    virtual bdd compute_support_conditions(const state*) const;
    virtual bdd compute_support_variables(const state*) const;
    virtual tgba_succ_iterator* succ_iter(const state* l,
					  const state*, const tgba*) const;

    virtual std::string
    transition_annotation(const tgba_succ_iterator* t) const;

    virtual void release_proviso(proviso*) const;

    // Check whether transition T is active, and if so return size().
    // If the transition T is inactive, return the number of an automaton
    // for which it isn't.
    unsigned is_active(const sync_state* q, const sync_transition* t) const;

    bool is_active(const sync_state* q, const action_vect* v) const;

    typedef std::list<sync_transition*> stlist;
    stlist E1UE2(const sync_transition* t) const;
    stlist E3(const sync_transition* t, unsigned i) const;

    friend class sync_transitions;
    friend class sync_part;
    friend class sync_transition_set_iterator;
    friend class sync_transition;
  };
}

#endif // SPOT_SYNC_HH
