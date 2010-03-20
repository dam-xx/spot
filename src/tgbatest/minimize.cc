// Copyright (C) 2010 Laboratoire de Recherche et Développement
// de l'Epita (LRDE).
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

#include <queue>
#include <cstring>
#include "ltlast/unop.hh"
#include "tgbaalgos/dotty.hh"
#include "tgbaalgos/minimize.hh"
#include "tgbaalgos/powerset.hh"
#include "ltlparse/public.hh"
#include "tgba/tgbaexplicit.hh"
#include "ltlparse/ltlfile.hh"
#include "tgbaalgos/ltl2tgba_fm.hh"

namespace spot
{
  unsigned tgba_size(const tgba* a)
  {
    typedef Sgi::hash_set<const state*,
      state_ptr_hash, state_ptr_equal> hash_type;
    hash_type seen;
    std::queue<state*> tovisit;
    // Perform breadth-first search.
    state* init = a->get_init_state();
    tovisit.push(init);
    seen.insert(init);
    unsigned count = 0;
    // While there are still states to visit.
    while (!tovisit.empty())
    {
      ++count;
      state* cur = tovisit.front();
      tovisit.pop();
      tgba_succ_iterator* sit = a->succ_iter(cur);
      for (sit->first(); !sit->done(); sit->next())
      {
        state* dst = sit->current_state();
        // Is it a new state ?
        if (seen.find(dst) == seen.end())
        {
          // Yes, register the successor for later processing.
          tovisit.push(dst);
          seen.insert(dst);
        }
        else
          // No, free dst.
          delete dst;
      }
      delete sit;
    }
    hash_type::iterator it2;
    // Free visited states.
    for (it2 = seen.begin(); it2 != seen.end(); it2++)
      delete *it2;
    return count;
  }
}

// Compare the sizes of the initial and the minimal automata.
void compare_automata(const spot::tgba* a,
                      const spot::tgba* min_a)
{
  unsigned init_size = spot::tgba_size(a);
  unsigned min_size = spot::tgba_size(min_a);
  unsigned diff_size = init_size - min_size;
  std::cout << init_size << " " << min_size << " " << diff_size << std::endl;
}

void usage(const char* prog)
{
  std::cout << "Usage: " << prog << " ltl_file" << std::endl
            << "       " << prog << " -f formula" << std::endl;
}

int main(int argc, char* argv[])
{
  if (argc == 1)
  {
    usage(argv[0]);
    return 1;
  }
  spot::bdd_dict* dict = new spot::bdd_dict();
  // Use the formula in argv[2]
  if (!strcmp(argv[1], "-f"))
  {
    if (argc != 3)
    {
      usage(argv[0]);
      return 1;
    }
    std::ofstream out_dot ("auto.dot");
    spot::ltl::parse_error_list pel;
    spot::ltl::formula* f = spot::ltl::parse(argv[2], pel);
    spot::tgba_explicit* a = ltl_to_tgba_fm(f, dict, true);
//    spot::tgba_explicit* det = spot::tgba_powerset(a);
    spot::tgba_explicit* res = minimize(a);
    compare_automata(a, res);
    spot::dotty_reachable(out_dot, res);
//    delete det;
    delete a;
    delete res;
    f->destroy();
  }
  else
  {
    // Read a serie of formulae in a file.
    spot::ltl::ltl_file formulae(argv[1]);
    spot::ltl::formula* f;
    while ((f = formulae.next()))
    {
      spot::ltl::formula* f_neg =
        spot::ltl::unop::instance(spot::ltl::unop::Not,
                                  f->clone());
      spot::tgba_explicit* a = ltl_to_tgba_fm(f_neg, dict, true);
      spot::tgba_explicit* res = minimize(a);
      compare_automata(a, res);
      delete a;
      delete res;
      f->destroy();
      f_neg->destroy();
    }
  }
  delete dict;
}
