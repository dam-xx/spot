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

#include "fairkripke.hh"

namespace spot
{

  fair_kripke_succ_iterator::fair_kripke_succ_iterator(const fair_kripke* aut,
						       const state* st)
    : cond(aut->conditions_of_state(st)),
      acc_cond(aut->acceptance_conditions_of_state(st))
  {
  }

  fair_kripke_succ_iterator::~fair_kripke_succ_iterator()
  {
  }

  bdd
  fair_kripke_succ_iterator::current_conditions() const
  {
    return cond;
  }

  bdd
  fair_kripke_succ_iterator::current_acceptance_conditions() const
  {
    return acc_cond;
  }

  bdd
  fair_kripke::compute_support_conditions(const state* s) const
  {
    return conditions_of_state(s);
  }

  bdd
  fair_kripke::compute_support_variables(const state* s) const
  {
    return bdd_support(conditions_of_state(s));
  }
}
