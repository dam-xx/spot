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
#include "eltlparse/public.hh"
#include "tgbaalgos/eltl2tgba_lacim.hh"
#include "tgbaalgos/dotty.hh"
#include "tgbaalgos/save.hh"
#include "ltlvisit/destroy.hh"

void
syntax(char* prog)
{
  std::cerr << "Usage: "<< prog << " [OPTIONS...] formula [file]" << std::endl
	    << "       "<< prog << " -F [OPTIONS...] file [file]" << std::endl
	    << std::endl
	    << "Options:" << std::endl
	    << "  -F    read the formula from the file (extended input format)"
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
G=(0 0 $0)";
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
  else
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

  spot::dotty_reachable(std::cout, concrete);
  if (argc >= formula_index + 1)
  {
    std::ofstream ofs(argv[formula_index + 1]);
    spot::tgba_save_reachable(ofs, concrete);
    ofs.close();
  }

  spot::ltl::destroy(f);
  delete concrete;
  delete dict;
}
