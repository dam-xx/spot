// Copyright (C) 2003, 2005  Laboratoire d'Informatique de Paris 6 (LIP6),
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
#include "ltlast/atomic_prop.hh"
#include "ltlvisit/randomltl.hh"
#include "ltlvisit/tostring.hh"
#include "ltlvisit/destroy.hh"
#include "ltlvisit/length.hh"
#include "ltlenv/defaultenv.hh"
#include "misc/random.hh"

void
syntax(char* prog)
{
  std::cerr << "Usage: "<< prog << " [OPTIONS...] PROPS..." << std::endl
	    << std::endl
	    << "Options:" << std::endl
	    << "  -d      dump priorities, do not generate any formula"
	    << std::endl
	    << "  -f N    the size of the formula [15]" << std::endl
	    << "  -F N    number of formulae to generate [1]" << std::endl
	    << "  -p S    priorities to use" << std::endl
	    << "  -s N    seed for the random number generator" << std::endl
	    << std::endl
	    << "Where:" << std::endl
	    << "  F      are floating values" << std::endl
	    << "  S      are `KEY=F, KEY=F, ...' strings" << std::endl
	    << "  N      are positive integers" << std::endl
	    << "  PROPS  are the atomic properties to use on transitions"
	    << std::endl
	    << "Use -d to see the list of KEYs." << std::endl;
  exit(2);
}

int
to_int(const char* s)
{
  char* endptr;
  int res = strtol(s, &endptr, 10);
  if (*endptr)
    {
      std::cerr << "Failed to parse `" << s << "' as an integer." << std::endl;
      exit(1);
    }
  return res;
}

int
main(int argc, char** argv)
{
  bool opt_d = false;
  int opt_s = 0;
  int opt_f = 15;
  int opt_F = 1;
  char* opt_p = 0;

  spot::ltl::environment& env(spot::ltl::default_environment::instance());
  spot::ltl::atomic_prop_set* ap = new spot::ltl::atomic_prop_set;

  int argn = 0;

  if (argc <= 1)
    syntax(argv[0]);

  while (++argn < argc)
    {
      if (!strcmp(argv[argn], "-d"))
	{
	  opt_d = true;
	}
      else if (!strcmp(argv[argn], "-f"))
	{
	  if (argc < argn + 2)
	    syntax(argv[0]);
	  opt_f = to_int(argv[++argn]);
	}
      else if (!strcmp(argv[argn], "-F"))
	{
	  if (argc < argn + 2)
	    syntax(argv[0]);
	  opt_F = to_int(argv[++argn]);
	}
      else if (!strcmp(argv[argn], "-p"))
	{
	  if (argc < argn + 2)
	    syntax(argv[0]);
	  opt_p = argv[++argn];
	}
      else if (!strcmp(argv[argn], "-s"))
	{
	  if (argc < argn + 2)
	    syntax(argv[0]);
	  opt_s = to_int(argv[++argn]);
	}
      else
	{
	  ap->insert(static_cast<spot::ltl::atomic_prop*>
		     (env.require(argv[argn])));
	}
    }

  spot::srand(opt_s);

  spot::ltl::random_ltl rl(ap);

  const char* tok = rl.parse_options(opt_p);
  if (tok)
    {
      std::cerr << "failed to parse probabilities near `"
		<< tok << "'" << std::endl;
      exit(2);
    }

  if (opt_d)
    {
      rl.dump_priorities(std::cout);
    }
  else
    {
      while (opt_F--)
	{
	  spot::ltl::formula* f = rl.generate(opt_f);
	  std::cout << spot::ltl::to_string(f) << std::endl;
	  assert(spot::ltl::length(f) <= opt_f);
	  spot::ltl::destroy(f);
	}
    }
  delete ap;
}
