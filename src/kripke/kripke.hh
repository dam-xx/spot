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

#ifndef SPOT_KRIPKE_KRIPKE_HH
# define SPOT_KRIPKE_KRIPKE_HH

#include "fairkripke.hh"

namespace spot
{

  typedef fair_kripke_succ_iterator kripke_succ_iterator;

  class kripke: public fair_kripke
  {
  public:
    virtual ~kripke();

    virtual bdd acceptance_conditions_of_state(const state* s) const;
    virtual bdd neg_acceptance_conditions() const;
    virtual bdd all_acceptance_conditions() const;
  };
}

#endif // SPOT_KRIPKE_KRIPKE_HH
