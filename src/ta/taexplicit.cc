// Copyright (C) 2010 Laboratoire de Recherche et Developpement
// de l Epita (LRDE).
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

#include "ltlast/atomic_prop.hh"
#include "ltlast/constant.hh"
#include "taexplicit.hh"
#include "tgba/formula2bdd.hh"
#include "misc/bddop.hh"
#include <cassert>
#include "ltlvisit/tostring.hh"

#include "tgba/bddprint.hh"

namespace spot
{

  ////////////////////////////////////////
  // ta_explicit_succ_iterator

  ta_explicit_succ_iterator::ta_explicit_succ_iterator(
      const state_ta_explicit* s) :
    source_(s)
  {
    transitions_ = s->get_transitions();
  }

  void
  ta_explicit_succ_iterator::first()
  {
    if (transitions_ != 0)
      i_ = transitions_->begin();
  }

  void
  ta_explicit_succ_iterator::next()
  {
    ++i_;
  }

  bool
  ta_explicit_succ_iterator::done() const
  {
    return transitions_ == 0 || i_ == transitions_->end();
  }

  state*
  ta_explicit_succ_iterator::current_state() const
  {
    assert(!done());
    state_ta_explicit* s = (*i_)->dest;
    return s;
  }

  bdd
  ta_explicit_succ_iterator::current_condition() const
  {
    assert(!done());
    return (*i_)->condition;
  }

  bool
  ta_explicit_succ_iterator::is_stuttering_transition() const
  {
    return source_->get_tgba_condition() == ((*i_)->dest)->get_tgba_condition();
  }

  ////////////////////////////////////////
  // state_ta_explicit

  state_ta_explicit::transitions*
  state_ta_explicit::get_transitions() const
  {
    return transitions_;
  }

  void
  state_ta_explicit::add_transition(state_ta_explicit::transition* t)
  {
    if (transitions_ == 0)
      transitions_ = new transitions;

    transitions_->push_back(t);

  }

  const state*
  state_ta_explicit::get_tgba_state() const
  {
    return tgba_state_;
  }

  const bdd
  state_ta_explicit::get_tgba_condition() const
  {
    return tgba_condition_;
  }

  bool
  state_ta_explicit::is_accepting_state() const
  {
    return is_accepting_state_;
  }

  bool
  state_ta_explicit::is_initial_state() const
  {
    return is_initial_state_;
  }

  void
  state_ta_explicit::set_accepting_state(bool is_accepting_state)
  {
    is_accepting_state_ = is_accepting_state;
  }

  bool
  state_ta_explicit::is_livelock_accepting_state() const
  {
    return is_livelock_accepting_state_;
  }

  void
  state_ta_explicit::set_livelock_accepting_state(
      bool is_livelock_accepting_state)
  {
    is_livelock_accepting_state_ = is_livelock_accepting_state;
  }

  void
  state_ta_explicit::set_initial_state(bool is_initial_state)
  {
    is_initial_state_ = is_initial_state;
  }

  int
  state_ta_explicit::compare(const spot::state* other) const
  {
    const state_ta_explicit* o = dynamic_cast<const state_ta_explicit*> (other);
    assert(o);

    int compare_value = tgba_state_->compare(o->tgba_state_);

    if (compare_value != 0)
      return compare_value;

    return tgba_condition_.id() - o->tgba_condition_.id();
  }

  size_t
  state_ta_explicit::hash() const
  {
    return wang32_hash(tgba_state_->hash()) ^ wang32_hash(tgba_condition_.id());
  }

  state_ta_explicit*
  state_ta_explicit::clone() const
  {
    return new state_ta_explicit(*this);
  }

  void
  state_ta_explicit::delete_stuttering_and_hole_successors()
  {
    state_ta_explicit::transitions* trans = get_transitions();
    state_ta_explicit::transitions::iterator it_trans;

    if (trans != 0)
      for (it_trans = trans->begin(); it_trans != trans->end();)
        {
          state_ta_explicit* dest = (*it_trans)->dest;
          bool is_stuttering_transition = (get_tgba_condition() == (dest)->get_tgba_condition());
          bool dest_is_livelock_accepting = dest->is_livelock_accepting_state();

          //Before deleting stuttering transitions, propaged back livelock and initial state's properties
          if(is_stuttering_transition){
              if(dest_is_livelock_accepting) set_livelock_accepting_state(true);
              if(dest->is_initial_state()) set_initial_state(true);
          }



          //remove hole successors states
          state_ta_explicit::transitions* dest_trans =
              (dest)->get_transitions();
          bool dest_trans_empty = dest_trans == 0 || dest_trans->empty();
          if (is_stuttering_transition || (dest_trans_empty && (!dest_is_livelock_accepting)) )
            {
              it_trans = trans->erase(it_trans);
            }
          else
            {
              it_trans++;
            }
        }

  }

  ////////////////////////////////////////
  // ta_explicit


  ta_explicit::ta_explicit(const tgba* tgba_) :
    tgba_(tgba_)
  {
  }

  ta_explicit::~ta_explicit()
  {
    ta::states_set_t::iterator it;
    for (it = states_set_.begin(); it != states_set_.end(); it++)
      {
        const state_ta_explicit* s =
            dynamic_cast<const state_ta_explicit*> (*it);
        state_ta_explicit::transitions* trans = s->get_transitions();
        state_ta_explicit::transitions::iterator it_trans;
        // We don't destroy the transitions in the state's destructor because
        // they are not cloned.
        if (trans != 0)
          for (it_trans = trans->begin(); it_trans != trans->end(); it_trans++)
            {
              delete *it_trans;
            }
        delete trans;
        delete s->get_tgba_state();
        delete s;
      }

  }

  state_ta_explicit*
  ta_explicit::add_state(state_ta_explicit* s)
  {
    std::pair<ta::states_set_t::iterator, bool> add_state_to_ta =
        states_set_.insert(s);

    return dynamic_cast<state_ta_explicit*> (*add_state_to_ta.first);

  }

  void
  ta_explicit::add_to_initial_states_set(state* state)
  {
    state_ta_explicit * s = dynamic_cast<state_ta_explicit*> (state);

    s->set_initial_state(true);

    initial_states_set_.insert(s);

  }

  void
  ta_explicit::delete_stuttering_and_hole_successors(spot::state* s)
  {
    state_ta_explicit * state = dynamic_cast<state_ta_explicit*> (s);
    state->delete_stuttering_and_hole_successors();

  }

  void
  ta_explicit::create_transition(state_ta_explicit* source, bdd condition,
      state_ta_explicit* dest)
  {
    state_ta_explicit::transition* t = new state_ta_explicit::transition;
    t->dest = dest;
    t->condition = condition;
    source->add_transition(t);

  }

  const ta::states_set_t*
  ta_explicit::get_initial_states_set() const
  {
    return &initial_states_set_;

  }

  bdd
  ta_explicit::get_state_condition(const spot::state* initial_state) const
  {
    const state_ta_explicit* sta =
        dynamic_cast<const state_ta_explicit*> (initial_state);
    return sta->get_tgba_condition();
  }

  bool
  ta_explicit::is_accepting_state(const spot::state* s) const
  {
    const state_ta_explicit* sta = dynamic_cast<const state_ta_explicit*> (s);
    return sta->is_accepting_state();
  }

  bool
  ta_explicit::is_initial_state(const spot::state* s) const
  {
    const state_ta_explicit* sta = dynamic_cast<const state_ta_explicit*> (s);
    return sta->is_initial_state();
  }

  bool
  ta_explicit::is_livelock_accepting_state(const spot::state* s) const
  {
    const state_ta_explicit* sta = dynamic_cast<const state_ta_explicit*> (s);
    return sta->is_livelock_accepting_state();
  }

  ta_succ_iterator*
  ta_explicit::succ_iter(const spot::state* state) const
  {
    const state_ta_explicit* s = dynamic_cast<const state_ta_explicit*> (state);
    assert(s);
    return new ta_explicit_succ_iterator(s);
  }

  bdd_dict*
  ta_explicit::get_dict() const
  {
    return tgba_->get_dict();
  }

  const tgba*
  ta_explicit::get_tgba() const
  {
    return tgba_;
  }

  std::string
  ta_explicit::format_state(const spot::state* s) const
  {
    const state_ta_explicit* sta = dynamic_cast<const state_ta_explicit*> (s);
    assert(sta);
    return tgba_->format_state(sta->get_tgba_state()) + "\n"
        + bdd_format_formula(get_dict(), sta->get_tgba_condition());

  }

  void
  ta_explicit::delete_stuttering_transitions()
  {
    ta::states_set_t::iterator it;
    for (it = states_set_.begin(); it != states_set_.end(); it++)
      {

        const state_ta_explicit* source =
            dynamic_cast<const state_ta_explicit*> (*it);

        state_ta_explicit::transitions* trans = source->get_transitions();
        state_ta_explicit::transitions::iterator it_trans;

        if (trans != 0)
          for (it_trans = trans->begin(); it_trans != trans->end();)
            {
              if (source->get_tgba_condition()
                  == ((*it_trans)->dest)->get_tgba_condition())
                {
                  delete *it_trans;
                  it_trans = trans->erase(it_trans);
                }
              else
                {
                  it_trans++;
                }
            }
      }

  }

  void
  ta_explicit::free_state(const spot::state*) const
  {
  }

}
