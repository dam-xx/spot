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
#include <sstream>
#include "ltlvisit/apcollect.hh"
#include "ltlvisit/destroy.hh"
#include "tgbaalgos/randomgraph.hh"
#include "tgbaalgos/save.hh"
#include "ltlenv/defaultenv.hh"
#include "ltlast/atomic_prop.hh"
#include "tgbaalgos/dotty.hh"
#include "misc/random.hh"

#include "tgbaalgos/emptiness.hh"
#include "tgbaalgos/gtec/gtec.hh"
#include "tgbaalgos/magic.hh"
#include "tgbaalgos/se05.hh"
#include "tgbaalgos/tau03.hh"
#include "tgbaalgos/tau03opt.hh"
#include "tgbaalgos/replayrun.hh"

void
syntax(char* prog)
{
  std::cerr << "Usage: "<< prog << " [OPTIONS...] PROPS..." << std::endl
	    << std::endl
	    << "Options:" << std::endl
	    << "  -a N F  number of acceptance conditions and probability that"
	    << " one is true [0 0.0]" << std::endl
	    << "  -d F    density of the graph [0.2]" << std::endl
	    << "  -e N    compare result of all "
	    << "emptiness checks on N randomly generated graphs" << std::endl
	    << "  -g      output in dot format" << std::endl
	    << "  -n N    number of nodes of the graph [20]" << std::endl
	    << "  -r      compute and replay acceptance runs (implies -e)"
	    << std::endl
	    << "  -s N    seed for the random number generator" << std::endl
	    << "  -t F    probability of the atomic propositions to be true"
	    << " [0.5]" << std::endl
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
  int opt_ec = 0;
  int opt_ec_seed = 0;
  bool opt_replay = false;

  int argn = 0;

  int exit_code = 0;

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
	  if (argc < argn + 2)
	    syntax(argv[0]);
	  opt_d = to_float(argv[++argn]);
	}
      else if (!strcmp(argv[argn], "-e"))
	{
	  if (argc < argn + 2)
	    syntax(argv[0]);
	  opt_ec = to_int(argv[++argn]);
	}
      else if (!strcmp(argv[argn], "-g"))
	{
	  opt_dot = true;
	}
      else if (!strcmp(argv[argn], "-n"))
	{
	  if (argc < argn + 2)
	    syntax(argv[0]);
	  opt_n = to_int(argv[++argn]);
	}
      else if (!strcmp(argv[argn], "-r"))
	{
	  opt_replay = true;
	  if (!opt_ec)
	    opt_ec = 1;
	}
      else if (!strcmp(argv[argn], "-s"))
	{
	  if (argc < argn + 2)
	    syntax(argv[0]);
	  opt_ec_seed = to_int(argv[++argn]);
	  spot::srand(opt_ec_seed);
	}
      else if (!strcmp(argv[argn], "-t"))
	{
	  if (argc < argn + 2)
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


  std::set<int> failed_seeds;
  do
    {
      if (opt_ec)
	{
	  std::cout << "seed: " << opt_ec_seed << std::endl;
	  spot::srand(opt_ec_seed);
	}

      spot::tgba* a = spot::random_graph(opt_n, opt_d, ap, dict,
					 opt_n_acc, opt_a, opt_t, &env);

      if (!opt_ec)
	{
	  if (opt_dot)
	    dotty_reachable(std::cout, a);
	  else
	    tgba_save_reachable(std::cout, a);
	}
      else
	{
	  std::vector<spot::emptiness_check*> ec_obj;
	  std::vector<std::string> ec_name;
	  std::vector<bool> ec_safe;

	  ec_obj.push_back(new spot::couvreur99_check(a));
	  ec_name.push_back("couvreur99");
	  ec_safe.push_back(true);

	  ec_obj.push_back(new spot::couvreur99_check_shy(a));
	  ec_name.push_back("couvreur99_shy");
	  ec_safe.push_back(true);

	  if (opt_n_acc >= 1)
	    {
              ec_obj.push_back(spot::explicit_tau03_search(a));
              ec_name.push_back("explicit_tau03_search");
              ec_safe.push_back(true);

              ec_obj.push_back(spot::explicit_tau03_opt_search(a));
              ec_name.push_back("explicit_tau03_opt_search");
              ec_safe.push_back(true);
            }

	  if (opt_n_acc <= 1)
	    {
	      ec_obj.push_back(spot::explicit_magic_search(a));
	      ec_name.push_back("explicit_magic_search");
	      ec_safe.push_back(true);

	      ec_obj.push_back(spot::bit_state_hashing_magic_search(a, 4096));
	      ec_name.push_back("bit_state_hashing_magic_search");
	      ec_safe.push_back(false);

	      ec_obj.push_back(spot::explicit_se05_search(a));
	      ec_name.push_back("explicit_se05");
	      ec_safe.push_back(true);

	      ec_obj.push_back(spot::bit_state_hashing_se05_search(a, 4096));
	      ec_name.push_back("bit_state_hashing_se05_search");
	      ec_safe.push_back(false);
	    }

	  int n_ec = ec_obj.size();
	  int n_empty = 0;
	  int n_non_empty = 0;
	  int n_maybe_empty = 0;

	  for (int i = 0; i < n_ec; ++i)
	    {
	      std::cout.width(32);
	      std::cout << ec_name[i] << ": ";
	      spot::emptiness_check_result* res = ec_obj[i]->check();
	      if (res)
		{
		  std::cout << "accepting run exists";
		  ++n_non_empty;
		  if (opt_replay)
		    {
		      spot::tgba_run* run = res->accepting_run();
		      if (run)
			{
			  std::ostringstream s;
			  if (!spot::replay_tgba_run(s, a, run))
			    {
			      std::cout << ", but could not replay it (ERROR!)";
			      failed_seeds.insert(opt_ec_seed);
			    }
			  else
			    {
			      std::cout << ", computed OK";
			    }
			  delete run;
			}
		      else
			{
			  std::cout << ", not computed";
			}
		    }
		  std::cout << std::endl;
		  delete res;
		}
	      else
		{
		  if (ec_safe[i])
		    {
		      std::cout << "empty language" << std::endl;
		      ++n_empty;
		    }
		  else
		    {
		      std::cout << "maybe empty language" << std::endl;
		      ++n_maybe_empty;
		    }

		}
	      delete ec_obj[i];
	    }

	  assert(n_empty + n_non_empty + n_maybe_empty == n_ec);

	  if ((n_empty == 0 && (n_non_empty + n_maybe_empty) != n_ec)
	      || (n_empty != 0 && n_non_empty != 0))
	    {
	      std::cout << "ERROR: not all algorithms agree" << std::endl;
	      failed_seeds.insert(opt_ec_seed);
	    }
	}

      delete a;

      if (opt_ec)
	{
	  --opt_ec;
	  ++opt_ec_seed;
	}
    }
  while (opt_ec);
  if (!failed_seeds.empty())
    {
      exit_code = 1;
      std::cout << "The check failed for the following seeds:";
      for (std::set<int>::const_iterator i = failed_seeds.begin();
	   i != failed_seeds.end(); ++i)
	std::cout << " " << *i;
      std::cout << std::endl;
    }

  for (spot::ltl::atomic_prop_set::iterator i = ap->begin();
       i != ap->end(); ++i)
    spot::ltl::destroy(*i);

  delete ap;
  delete dict;
  return exit_code;
}
