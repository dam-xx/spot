// Copyright (C) 2003  Laboratoire d'Informatique de Paris 6 (LIP6),
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
#include <cassert>
#include "ltlparse/public.hh"
#include "ltlvisit/lunabbrev.hh"
#include "ltlvisit/tunabbrev.hh"
#include "ltlvisit/dump.hh"
#include "ltlvisit/nenoform.hh"
#include "ltlvisit/destroy.hh"
#include "ltlvisit/tostring.hh"
#include "ltlvisit/reducform.hh"
#include "ltlast/allnodes.hh"

void
syntax(char* prog)
{
  std::cerr << prog << " formula1 (formula2)?" << std::endl;
  exit(2);
}

typedef spot::ltl::formula*  ptrfunct(const spot::ltl::formula*);

int
main(int argc, char** argv)
{
  if (argc < 2)
    syntax(argv[0]);
  
  spot::ltl::parse_error_list p1;
  spot::ltl::formula* f1 = spot::ltl::parse(argv[1], p1);
  spot::ltl::formula* f2 = NULL;

  if (spot::ltl::format_parse_errors(std::cerr, argv[1], p1))
    return 2;

  
  if (argc == 3){
    spot::ltl::parse_error_list p2;
    f2 = spot::ltl::parse(argv[2], p2);
    if (spot::ltl::format_parse_errors(std::cerr, argv[2], p2))
      return 2;
  }
  
  int exit_code = 0;
  
  spot::ltl::formula* ftmp1;
  spot::ltl::formula* ftmp2;
  
  ftmp1 = f1;
  f1 = unabbreviate_logic(f1);
  ftmp2 = f1;
  f1 = negative_normal_form(f1);
  spot::ltl::destroy(ftmp1);
  spot::ltl::destroy(ftmp2);
  
  
  int length_f1_before = spot::ltl::form_length(f1);
  std::string f1s_before = spot::ltl::to_string(f1);
  
  ftmp1 = f1;
  f1 = spot::ltl::reduce(f1);
  ftmp2 = f1;
  f1 = spot::ltl::unabbreviate_logic(f1);
  spot::ltl::destroy(ftmp1);
  spot::ltl::destroy(ftmp2);
  
  int length_f1_after = spot::ltl::form_length(f1);
  std::string f1s_after = spot::ltl::to_string(f1);

  bool red = (length_f1_after < length_f1_before);
  if (red);
  std::string f2s = "";
  if (f2 != NULL) {
    ftmp1 = f2;
    f2 = unabbreviate_logic(f2);
    ftmp2 = f2;
    f2 = negative_normal_form(f2);
    spot::ltl::destroy(ftmp1);
    spot::ltl::destroy(ftmp2);
    ftmp1 = f2;
    f2 = unabbreviate_logic(f2);
    spot::ltl::destroy(ftmp1);
    f2s = spot::ltl::to_string(f2);
  }
  
  
  if (red && (f2 == NULL)) {
    std::cout << length_f1_before << " " << length_f1_after 
	      << " '" << f1s_before << "' reduce to '" << f1s_after << "'" << std::endl;
  }
    
  if (f2 != NULL){
    if (f1 != f2) {
      if (length_f1_after < length_f1_before)
	std::cout << f1s_before << " ** " << f2s << " ** " << f1s_after << " KOREDUC " << std::endl;
      else
	std::cout << f1s_before << " ** " << f2s << " ** " << f1s_after << " KOIDEM " << std::endl;
      exit_code = 1;
    }
    else {
      if (f1s_before != f1s_after)
	std::cout << f1s_before << " ** " << f2s << " ** " << f1s_after << " OKREDUC " << std::endl;
      else
	std::cout << f1s_before << " ** " << f2s << " ** " << f1s_after << " OKIDEM" << std::endl;
      exit_code = 0;
    }
  }
  else{  
    if (length_f1_after < length_f1_before) exit_code = 0;
  }
  
  /*
  spot::ltl::dump(std::cout, f1); std::cout << std::endl;
  if (f2 != NULL)
    spot::ltl::dump(std::cout, f2); std::cout << std::endl;
  */
  
  
  spot::ltl::destroy(f1);
  if (f2 != NULL)
    spot::ltl::destroy(f2);
  

  assert(spot::ltl::atomic_prop::instance_count() == 0);
  assert(spot::ltl::unop::instance_count() == 0);
  assert(spot::ltl::binop::instance_count() == 0);
  assert(spot::ltl::multop::instance_count() == 0);
  
  return exit_code;
}
