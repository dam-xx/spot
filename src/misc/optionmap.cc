// Copyright (C) 2005  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#include <cstring>
#include <iostream>
#include "optionmap.hh"

namespace spot
{
  namespace
  {
    bool
    to_int(const char* s, int &i)
    {
      char* endptr;
      int res = strtol(s, &endptr, 10);
      if (*endptr)
	return false;
      i = res;
      return true;
    }
  };

  const char*
  option_map::parse_options(char* options)
  {
    char* opt = strtok(options, ", \t;");
    while (opt)
      {
	char* equal = strchr(opt, '=');
	if (equal)
	  {
	    *equal = 0;
	    int val;
	    if (!to_int(equal + 1, val))
	      return opt;
	    options_[opt] = val;
	  }
	else
	  {
	    options_[opt] = 1;
	  }
	opt = strtok(0, ", \t;");
      }
    return 0;
  }

  int
  option_map::get(const char* option, int def) const
  {
    std::map<std::string, int>::const_iterator it = options_.find(option);
    if (it == options_.end())
      // default value if not declared
      return def;
    else
      return it->second;
  }

  int option_map::operator[](const char* option) const
  {
    return get(option);
  }

  int
  option_map::set(const char* option, int val, int def)
  {
    int old = get(option, def);
    options_[option] = val;
    return old;
  }

  int&
  option_map::operator[](const char* option)
  {
    return options_[option];
  }

  std::ostream&
  operator<<(std::ostream& os, const option_map& m)
  {
    for (std::map<std::string, int>::const_iterator it = m.options_.begin();
	 it != m.options_.end(); ++it)
      os << "\"" << it->first << "\" = " << it->second << std::endl;
    return os;
  }
};
