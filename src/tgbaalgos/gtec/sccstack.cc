// Copyright (C) 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#include "sccstack.hh"

namespace spot
{
  scc_stack::connected_component::connected_component(int i)
  {
    index = i;
    condition = bddfalse;
  }

  scc_stack::connected_component&
  scc_stack::top()
  {
    return s.top();
  }

  void
  scc_stack::pop()
  {
    s.pop();
  }

  void
  scc_stack::push(int index)
  {
    s.push(connected_component(index));
  }

  size_t
  scc_stack::size() const
  {
    return s.size();
  }

  bool
  scc_stack::empty() const
  {
    return s.empty();
  }
}
