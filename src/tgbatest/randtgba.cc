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

#include <cstdlib>
#include <iostream>
#include "ltlvisit/apcollect.hh"
#include "ltlvisit/destroy.hh"
#include "tgbaalgos/randomgraph.hh"
#include "tgbaalgos/save.hh"
#include "ltlenv/defaultenv.hh"
#include "ltlast/atomic_prop.hh"
#include "tgbaalgos/dotty.hh"
#include "misc/random.hh"

void
syntax(char* prog)
{
  std::cerr << "Usage: "<< prog << " [OPTIONS...] PROPS..." << std::endl
	    << std::endl
	    << "Options:" << std::endl
	    << "  -a N F  number of accepence conditions a propability that"
	    << " one is true [0 0.0]" << std::endl
	    << "  -d F    density of the graph [0.2]" << std::endl
	    << "  -g      output in dot format" << std::endl
	    << "  -n N    number of nodes of the graph [20]" << std::endl
	    << "  -s N    seed for the random number generator" << std::endl
	    << "  -t F    probability of the atomic propositions to be true"
	    << " [0.5]"
	    << std::endl
	    << "Where:" << std::endl
	    << "  F      are floats between 0.0 and 1.0 inclusive" << std::endl
	    << "  N      are positive integers" << std::endl
	    << "  PROPS  are the atomic properties to use on transitions"
	    << std::endl;
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

float
to_float(const char* s)
{
  char* endptr;
  float res = strtof(s, &endptr);
  if (*endptr)
    {
      std::cerr << "Failed to parse `" << s << "' as a float." << std::endl;
      exit(1);
    }
  return res;
}

int
main(int argc, char** argv)
{
  int opt_n_acc = 0;
  float opt_a = 0.0;
  float opt_d = 0.2;
  int opt_n = 20;
  float opt_t = 0.5;

  bool opt_dot = false;

  int argn = 0;

  spot::ltl::environment& env(spot::ltl::default_environment::instance());
  spot::ltl::atomic_prop_set* ap = new spot::ltl::atomic_prop_set;

  if (argc <= 1)
    syntax(argv[0]);

  while (++argn < argc)
    {
      if (!strcmp(argv[argn], "-a"))
	{
	  if (argc < argn + 3)
	    syntax(argv[0]);
	  opt_n_acc = to_int(argv[++argn]);
	  opt_a = to_float(argv[++argn]);
	}
      else if (!strcmp(argv[argn], "-d"))
	{
	  if (argc < argn + 1)
	    syntax(argv[0]);
	  opt_d = to_float(argv[++argn]);
	}
      else if (!strcmp(argv[argn], "-g"))
	{
	  opt_dot = true;
	}
      else if (!strcmp(argv[argn], "-n"))
	{
	  if (argc < argn + 1)
	    syntax(argv[0]);
	  opt_n = to_int(argv[++argn]);
	}
      else if (!strcmp(argv[argn], "-s"))
	{
	  if (argc < argn + 1)
	    syntax(argv[0]);
	  spot::srand(to_int(argv[++argn]));
	}
      else if (!strcmp(argv[argn], "-t"))
	{
	  if (argc < argn + 1)
	    syntax(argv[0]);
	  opt_t = to_float(argv[++argn]);
	}
      else
	{
	  ap->insert(static_cast<spot::ltl::atomic_prop*>
		     (env.require(argv[argn])));
	}
    }

  spot::bdd_dict* dict = new spot::bdd_dict();

  spot::tgba* a = spot::random_graph(opt_n, opt_d, ap, dict,
				     opt_n_acc, opt_a, opt_t, &env);

  if (opt_dot)
    dotty_reachable(std::cout, a);
  else
    tgba_save_reachable(std::cout, a);

  for (spot::ltl::atomic_prop_set::iterator i = ap->begin();
       i != ap->end(); ++i)
    spot::ltl::destroy(*i);

  delete a;
  delete dict;
  delete ap;
  return 0;
}
