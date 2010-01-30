// Copyright (C) 2009, 2010 Laboratoire de Recherche et Développement
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

#ifndef SPOT_TGBA_TGBAFROMFILE_HH
# define SPOT_TGBA_TGBAFROMFILE_HH
#include <iosfwd>
#include <fstream>
#include "tgba/public.hh"
#include "tgbaalgos/ltl2tgba_fm.hh"
#include "tgbaalgos/save.hh"
#include "ltlparse/public.hh"

namespace spot
{
  class ltl_file
  {
  public:
    ltl_file(const char* filename, bdd_dict* dict);
    ~ltl_file();
    bool done();
    void next();
    void begin();
    tgba* current_automaton();
    std::string current_formula_string();
    ltl::formula* current_formula();
  private:
    bool done_;
    std::string formula_;
    ltl::formula* f_;
    std::ifstream input_file_;
    ltl::parse_error_list pel_;
    ltl::environment* env_;
    bdd_dict* dict_;
    bool fm_exprop_opt_;
    bool fm_symb_merge_opt_;
    bool post_branching_;
    bool fair_loop_approx_;
    ltl::atomic_prop_set* unobservables_;
    int fm_red_;
  };
}


#endif // SPOT_TGBA_TGBAFROMFILE_HH
