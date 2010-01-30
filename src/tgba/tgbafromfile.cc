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

#include "tgba/public.hh"
#include "tgbafromfile.hh"

namespace spot
{
  ltl_file::ltl_file(const char* filename, bdd_dict* dict)
  {
    input_file_.open(filename);
    assert(input_file_.is_open());
    done_ = false;
    env_ = &(ltl::default_environment::instance());
    dict_ = dict;
    f_ = 0;
    fm_exprop_opt_ = false;
    fm_symb_merge_opt_ = true;
    post_branching_ = false;
    fair_loop_approx_ = false;
    unobservables_ = 0;
    fm_red_ = ltl::Reduce_None;
  }

  ltl_file::~ltl_file()
  {
    input_file_.close();
  }

  bool ltl_file::done()
  {
    return(input_file_.eof());
  }

  void ltl_file::next()
  {
    if (f_)
    {
      f_->destroy();
      f_ = 0;
    }
    std::string line = "";
    while (line == "" && !input_file_.eof())
      getline(input_file_, line);
    formula_ = line;
  }

  void ltl_file::begin()
  {
    next();
  }

  tgba* ltl_file::current_automaton()
  {
    spot::tgba* a = 0;
    f_ = spot::ltl::parse(formula_, pel_, *env_, false);
    if (spot::ltl::format_parse_errors(std::cerr, formula_, pel_))
      return 0;
    // Generate the automaton corresponding to the formula
    a = spot::ltl_to_tgba_fm(f_, dict_, fm_exprop_opt_,
			     fm_symb_merge_opt_, post_branching_,
			     fair_loop_approx_, unobservables_,
			     fm_red_);
    return a;
  }

  ltl::formula* ltl_file::current_formula()
  {
    return f_;
  }

  std::string ltl_file::current_formula_string()
  {
    return formula_;
  }
}
