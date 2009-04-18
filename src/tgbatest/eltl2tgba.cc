// Copyright (C) 2008 Laboratoire d'Informatique de Paris 6 (LIP6),
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

#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>
#include <cstring>
#include "ltlparse/public.hh"
#include "eltlparse/public.hh"
#include "tgbaalgos/eltl2tgba_lacim.hh"
#include "tgbaalgos/dotty.hh"
#include "tgbaalgos/lbtt.hh"
#include "tgbaalgos/save.hh"
#include "ltlvisit/destroy.hh"
#include "ltlvisit/tostring.hh"

void
syntax(char* prog)
{
  std::cerr << "Usage: " << prog << " [OPTIONS...] formula [file]" << std::endl
	    << "       " << prog << " -F [OPTIONS...] file [file]" << std::endl
	    << "       " << prog << " -L [OPTIONS...] file [file]" << std::endl
	    << std::endl
	    << "Options:" << std::endl
	    << "  -F    read the formula from the file (extended input format)"
	    << std::endl
	    << "  -L    read the formula from an LBTT-compatible file"
	    << std::endl;
}

std::string
ltl_defs()
{
  std::string s = "\
X=(0 1 true	   \
   1 2 $0	   \
   accept 2)	   \
U=(0 0 $0	   \
   0 1 $1	   \
   accept 1)	   \
G=(0 0 $0)	   \
F=U(true, $0)	   \
R=!U(!$0, !$1)";
  return s;
}

int
main(int argc, char** argv)
{
  if (argc < 2)
  {
    syntax(argv[0]);
    return 1;
  }

  spot::eltl::parse_error_list p;
  spot::ltl::environment& env(spot::ltl::default_environment::instance());
  spot::ltl::formula* f = 0;
  int formula_index = 0;

  if (strcmp(argv[1], "-F") == 0)
  {
    f = spot::eltl::parse_file(argv[2], p, env, false);
    formula_index = 2;
  }
  if (strcmp(argv[1], "-L") == 0)
  {
    std::string input;
    std::ifstream ifs(argv[2]);
    std::getline(ifs, input, '\0');

    spot::ltl::parse_error_list p_;
    f = spot::ltl::parse(input, p_, env, false);
    input = ltl_defs();
    input += "%";
    input += spot::ltl::to_string(f, true);
    spot::ltl::destroy(f);

    f = spot::eltl::parse_string(input, p, env, false);
    formula_index = 2;
  }
  if (formula_index == 0)
  {
    std::stringstream oss;
    oss << ltl_defs() << "%" << argv[1];
    f = spot::eltl::parse_string(oss.str(), p, env, false);
    formula_index = 1;
  }

  if (spot::eltl::format_parse_errors(std::cerr, p))
  {
    if (f != 0)
      std::cout << f->dump() << std::endl;
    return 1;
  }

  assert(f != 0);
  std::cerr << f->dump() << std::endl;

  spot::bdd_dict* dict = new spot::bdd_dict();
  spot::tgba_bdd_concrete* concrete = spot::eltl_to_tgba_lacim(f, dict);

  if (strcmp(argv[1], "-L") == 0)
    spot::lbtt_reachable(std::cout, concrete);
  else
  {
    spot::dotty_reachable(std::cout, concrete);
    if (formula_index + 1 < argc)
    {
      std::ofstream ofs(argv[formula_index + 1]);
      spot::tgba_save_reachable(ofs, concrete);
      ofs.close();
    }
  }

  spot::ltl::destroy(f);
  delete concrete;
  delete dict;
}
