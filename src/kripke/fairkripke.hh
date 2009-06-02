// Copyright (C) 2009 Laboratoire de Recherche et Developpement de l'Epita
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

#ifndef SPOT_KRIPKE_FAIRKRIPKE_HH
# define SPOT_KRIPKE_FAIRKRIPKE_HH

#include "tgba/tgba.hh"
#include "tgba/succiter.hh"

namespace spot
{
  class fair_kripke;

  class fair_kripke_succ_iterator : public tgba_succ_iterator
  {
  public:
    fair_kripke_succ_iterator(const fair_kripke* aut, const state* st);
    virtual ~fair_kripke_succ_iterator();

    virtual bdd current_conditions() const;
    virtual bdd current_acceptance_conditions() const;
  protected:
    bdd cond;
    bdd acc_cond;
  };

  class fair_kripke : public tgba
  {
  public:
    virtual bdd conditions_of_state(const state* s) const = 0;
    virtual bdd acceptance_conditions_of_state(const state* s) const = 0;
    virtual fair_kripke_succ_iterator* kripke_succ_iter(const state* s) = 0;

    virtual fair_kripke_succ_iterator*
    succ_iter(const state* local_state,
	      const state* global_state = 0,
	      const tgba* global_automaton = 0);
  protected:
    virtual bdd compute_support_conditions(const state* s) const;
    virtual bdd compute_support_variables(const state* s) const;
  };

}


#endif // SPOT_KRIPKE_FAIRKRIPKE_HH
