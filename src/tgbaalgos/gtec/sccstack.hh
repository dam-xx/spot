// Copyright (C) 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
// d�partement Syst�mes R�partis Coop�ratifs (SRC), Universit� Pierre
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

#ifndef SPOT_TGBAALGOS_GTEC_SCCSTACK_HH
# define SPOT_TGBAALGOS_GTEC_SCCSTACK_HH

#include <stack>
#include <bdd.h>

namespace spot
{
  // A stack of Strongly-Connected Components, as needed by the
  // Tarjan-Couvreur algorithm.
  class scc_stack
  {
  public:
    struct connected_component
    {
    public:
      connected_component(int index = -1);

      /// Index of the SCC.
      int index;
      /// The bdd condition is the union of all acceptance conditions of
      /// transitions which connect the states of the connected component.
      bdd condition;
    };

    /// Stack a new SCC with index \a index.
    void push(int index);

    /// Access the top SCC.
    connected_component& top();

    /// Pop the top SCC.
    void pop();

    /// How many SCC are in stack.
    size_t size() const;

    /// Is the stack empty?
    bool empty() const;

    typedef std::stack<connected_component> stack_type;
    stack_type s;
  };
}

#endif // SPOT_TGBAALGOS_GTEC_SCCSTACK_HH