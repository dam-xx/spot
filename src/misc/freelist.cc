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

#include "freelist.hh"
#include <cassert>
#include <iostream>

namespace spot
{

  free_list::~free_list()
  {
  }

  int
  free_list::register_n(int n)
  {
    // Browse the free list until we find N consecutive variables.  We
    // try not to fragment the list my allocating the variables in the
    // smallest free range we find.
    free_list_type::iterator best = fl.end();
    free_list_type::iterator cur;
    for (cur = fl.begin(); cur != fl.end(); ++cur)
      {
	if (cur->second < n)
	  continue;
	if (n == cur->second)
	  {
	    best = cur;
	    break;
	  }
	if (best == fl.end()
	    || cur->second < best->second)
	  best = cur;
      }

    // We have found enough free variables.
    if (best != fl.end())
      {
	int result = best->first;
	best->second -= n;
	assert(best->second >= 0);
	// Erase the range if it's now empty.
	if (best->second == 0)
	  fl.erase(best);
	else
	  best->first += n;
	return result;
      }

    // We haven't found enough adjacent free variables;
    // ask for some more.
    return extend(n);
  }

  void
  free_list::release_n(int base, int n)
  {
    free_list_type::iterator cur;
    int end = base + n;
    for (cur = fl.begin(); cur != fl.end(); ++cur)
      {
	// Append to a range ...
	if (cur->first + cur->second == base)
	  {
	    cur->second += n;
	    // Maybe the next item on the list can be merged.
	    free_list_type::iterator next = cur;
	    ++next;
	    if (next != fl.end() && next->first == end)
	      {
		cur->second += next->second;
		fl.erase(next);
	      }
	    return;
	  }
	// ... or prepend to a range ...
	if (cur->first == end)
	  {
	    cur->first -= n;
	    cur->second += n;
	    return;
	  }
	// ... or insert a new range.
	if (cur->first > end)
	  break;
      }
    fl.insert(cur, pos_lenght_pair(base, n));
  }

  std::ostream&
  free_list::dump_free_list(std::ostream& os) const
  {
    free_list_type::const_iterator i;
    for (i = fl.begin(); i != fl.end(); ++i)
      os << "  (" << i->first << ", " << i->second << ")";
    return os;
  }

}
