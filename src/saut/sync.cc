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

#include "sync.hh"
#include <iostream>
#include <iterator>

namespace spot
{
  sync::sync(saut_list& sautlist)
    : auts(sautlist.begin(), sautlist.end())
  {
    assert(!sautlist.empty());

    std::cerr << "sync " << this << " created for (";
    autvec::iterator i = auts.begin();
    for (;;)
      {
	std::cerr << *i;
	if (++i == auts.end())
	  break;
	std::cerr << ", ";
      }
    std::cerr << ")" << std::endl;
  }

  bool
  sync::known_action(unsigned aut_num, const saut::action_name& act) const
  {
    assert(aut_num < auts.size());
    return auts[aut_num]->known_action(act);
  }

  bool
  sync::declare_rule(action_list& l)
  {
    assert(l.size() == auts.size());

    std::cerr << "sync " << this << " declares rule (";
    action_list::const_iterator i = l.begin();
    unsigned k = size();
    unsigned j = 0;
    bool not_epsilon = false;
    for (;;)
      {
	assert(i != l.end());
	const saut::action* a;
	if (*i)
	  {
	    a = auts[j]->known_action(**i);
	    assert(a);
	    not_epsilon = true;
	  }
	else
	  {
	    a = 0;
	  }
	std::cerr << a;
	if (++j >= k)
	  break;
	++i;
	std::cerr << ", ";
      }
    std::cerr << ")" << std::endl;
    return !not_epsilon;
  }

}
