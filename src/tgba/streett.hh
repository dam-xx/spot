// Copyright (C) 2006 Laboratoire d'Informatique de Paris 6 (LIP6),
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

#ifndef SPOT_TGBA_STREETT_HH
# define SPOT_TGBA_STREETT_HH

#include <bdd.h>
#include <list>

namespace spot
{
  struct streett_pair
  {
    bdd l;
    bdd u;
    streett_pair(bdd l, bdd u) : l(l), u(u) {}
  };

  class streett_acceptance_conditions
  {
  public:
    typedef std::list<streett_pair> acc_list;
    virtual const acc_list& get_streett_acceptance_conditions() const = 0;
    virtual ~streett_acceptance_conditions() {};
  };

  class streett_acceptance_conditions_stored
    : public streett_acceptance_conditions
  {
  protected:
    acc_list streett_acc;
    virtual const acc_list& get_streett_acceptance_conditions() const;
    virtual ~streett_acceptance_conditions_stored();
  };
}

#endif // SPOT_TGBA_STREETT_HH
