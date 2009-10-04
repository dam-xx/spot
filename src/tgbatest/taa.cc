// Copyright (C) 2009 Laboratoire d'Informatique de Paris
// 6 (LIP6), département Systèmes Répartis Coopératifs (SRC),
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

#include <iostream>
#include <cassert>
#include "misc/hash.hh"
#include "ltlenv/defaultenv.hh"
#include "ltlast/allnodes.hh"
#include "tgba/taa.hh"

typedef Sgi::hash_set<
  const spot::state*, spot::state_ptr_hash, spot::state_ptr_equal
  > mset;

void
dfs(spot::taa* a, const spot::state* s, mset& m)
{
  if (m.find(s) != m.end())
    return;
  m.insert(s);

  spot::tgba_succ_iterator* i = a->succ_iter(s);
  assert(i);
  for (i->first(); !i->done(); i->next())
  {
    std::cout << a->format_state(i->current_state()) << std::endl;
    dfs(a, i->current_state(), m);
  }
  delete i;
}

int
main()
{
  spot::bdd_dict* dict = new spot::bdd_dict();

  spot::ltl::default_environment& e =
    spot::ltl::default_environment::instance();
  spot::taa* a = new spot::taa(dict);

  typedef spot::taa::transition trans;
  typedef spot::taa::state state;
  typedef spot::taa::state_set state_set;

  std::string ss1_values[] = { "state 2", "state 3" };
  std::vector<std::string> ss1_vector(ss1_values, ss1_values + 2);
  trans* t1 = a->create_transition("state 1", ss1_vector);
  trans* t2 = a->create_transition("state 2", "state 3");
  trans* t3 = a->create_transition("state 2", "state 4");

  a->add_condition(t1, e.require("a"));
  a->add_condition(t2, e.require("b"));
  a->add_condition(t3, e.require("c"));

  mset m;
  spot::state* init = a->get_init_state();
  dfs(a, init, m);

  delete init;
  delete a;
  delete dict;
}
