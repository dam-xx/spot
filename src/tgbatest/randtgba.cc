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
#include <iomanip>
#include <sstream>
#include <utility>
#include "ltlvisit/apcollect.hh"
#include "ltlvisit/destroy.hh"
#include "tgbaalgos/randomgraph.hh"
#include "tgbaalgos/save.hh"
#include "ltlenv/defaultenv.hh"
#include "ltlast/atomic_prop.hh"
#include "tgbaalgos/dotty.hh"
#include "misc/random.hh"
#include "tgba/tgbatba.hh"

#include "tgbaalgos/emptiness.hh"
#include "tgbaalgos/emptiness_stats.hh"
#include "tgbaalgos/gtec/gtec.hh"
#include "tgbaalgos/gv04.hh"
#include "tgbaalgos/magic.hh"
#include "tgbaalgos/minimizerun.hh"
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
	    << " one is true" << std::endl
	    << "            [0 0.0]" << std::endl
	    << "  -d F    density of the graph [0.2]" << std::endl
	    << "  -D      degeneralize TGBA for emptiness-check algorithms that"
	    << " would" << std::endl
	    << "            otherwise be skipped (implies -e)" << std::endl
	    << "  -e N    compare result of all "
	    << "emptiness checks on N randomly generated graphs" << std::endl
	    << "  -g      output in dot format" << std::endl
	    << "  -m      try to minimize runs, in a second pass (implies -r)"
	    << std::endl
	    << "  -n N    number of nodes of the graph [20]" << std::endl
	    << "  -r      compute and replay acceptance runs (implies -e)"
	    << std::endl
	    << "  -s N    seed for the random number generator" << std::endl
	    << "  -t F    probability of the atomic propositions to be true"
	    << " [0.5]" << std::endl
	    << "  -z      display statistics about computed accepting runs"
	    << std::endl
	    << "  -Z      like -z, but print extra statistics after the run"
	    << " of each algorithm" << std::endl
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
  // Do not use strtof(), it does not exist on Solaris 9.
  float res = strtod(s, &endptr);
  if (*endptr)
    {
      std::cerr << "Failed to parse `" << s << "' as a float." << std::endl;
      exit(1);
    }
  return res;
}

struct ec_stat
{
  int min_states;
  int max_states;
  int tot_states;
  int min_transitions;
  int max_transitions;
  int tot_transitions;
  int min_max_depth;
  int max_max_depth;
  int tot_max_depth;
  int n;

  ec_stat()
    : n(0)
  {
  }

  void
  count(const spot::ec_statistics* ec)
  {
    int s = ec->states();
    int t = ec->transitions();
    int m = ec->max_depth();
    if (n++)
      {
	min_states = std::min(min_states, s);
	max_states = std::max(max_states, s);
	tot_states += s;
	min_transitions = std::min(min_transitions, t);
	max_transitions = std::max(max_transitions, t);
	tot_transitions += t;
	min_max_depth = std::min(min_max_depth, m);
	max_max_depth = std::max(max_max_depth, m);
	tot_max_depth += m;
      }
    else
      {
	min_states = max_states = tot_states = s;
	min_transitions = max_transitions = tot_transitions = t;
	min_max_depth = max_max_depth = tot_max_depth = m;
      }
  }
};

struct ar_stat
{
  int min_prefix;
  int max_prefix;
  int tot_prefix;
  int min_cycle;
  int max_cycle;
  int tot_cycle;
  int min_run;
  int max_run;
  int n;

  ar_stat()
    : n(0)
  {
  }

  void
  count(const spot::tgba_run* run)
  {
    int p = run->prefix.size();
    int c = run->cycle.size();
    if (n++)
      {
	min_prefix = std::min(min_prefix, p);
	max_prefix = std::max(max_prefix, p);
	tot_prefix += p;
	min_cycle = std::min(min_cycle, c);
	max_cycle = std::max(max_cycle, c);
	tot_cycle += c;
	min_run = std::min(min_run, c + p);
	max_run = std::max(max_run, c + p);
      }
    else
      {
	min_prefix = max_prefix = tot_prefix = p;
	min_cycle = max_cycle = tot_cycle = c;
	min_run = max_run = c + p;
      }
  }
};

typedef std::map<std::string, ec_stat> ec_stats_type;
ec_stats_type ec_stats;

typedef std::map<std::string, ar_stat> ar_stats_type;
ar_stats_type ar_stats;		// Statistics about accepting runs.
ar_stats_type mar_stats;        // ... about minimized accepting runs.

void
print_ar_stats(ar_stats_type& ar_stats)
{
      std::cout << std::setw(32) << ""
		<< " |       prefix      |       cycle       |"
		<< std::endl << std::setw(32) << "algorithm"
		<< " |  min < mean < max |  min < mean < max |   n "
		<< std::endl
		<< std::setw(79) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl << std::setprecision(3);
      for (ar_stats_type::const_iterator i = ar_stats.begin();
	   i != ar_stats.end(); ++i)
	std::cout << std::setw(32) << i->first << " |"
		  << std::setw(5) << i->second.min_prefix
		  << " "
		  << std::setw(6)
		  << static_cast<float>(i->second.tot_prefix) / i->second.n
		  << " "
		  << std::setw(5) << i->second.max_prefix
		  << " |"
		  << std::setw(5) << i->second.min_cycle
		  << " "
		  << std::setw(6)
		  << static_cast<float>(i->second.tot_cycle) / i->second.n
		  << " "
		  << std::setw(5) << i->second.max_cycle
		  << " |"
		  << std::setw(5) << i->second.n
		  << std::endl;
      std::cout << std::setw(79) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl
		<< std::setw(32) << ""
		<< " |        runs       |       total      |"
		<< std::endl << std::setw(32) << "algorithm"
		<< " |  min < mean < max | pre.  cyc.  runs |   n "
		<< std::endl
		<< std::setw(79) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl;
      for (ar_stats_type::const_iterator i = ar_stats.begin();
	   i != ar_stats.end(); ++i)
	std::cout << std::setw(32) << i->first << " |"
		  << std::setw(5)
		  << i->second.min_run
		  << " "
		  << std::setw(6)
		  << static_cast<float>(i->second.tot_prefix
					+ i->second.tot_cycle) / i->second.n
		  << " "
		  << std::setw(5)
		  << i->second.max_run
		  << " |"
		  << std::setw(5) << i->second.tot_prefix
		  << " "
		  << std::setw(5) << i->second.tot_cycle
		  << " "
		  << std::setw(5) << i->second.tot_prefix + i->second.tot_cycle
		  << " |"
		  << std::setw(5) << i->second.n
		  << std::endl;
}

int
main(int argc, char** argv)
{
  int opt_n_acc = 0;
  float opt_a = 0.0;
  float opt_d = 0.2;
  int opt_n = 20;
  float opt_t = 0.5;

  bool opt_z = false;
  bool opt_Z = false;

  bool opt_dot = false;
  int opt_ec = 0;
  int opt_ec_seed = 0;
  bool opt_minim = false;
  bool opt_replay = false;
  bool opt_degen = false;
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
      else if (!strcmp(argv[argn], "-D"))
	{
	  opt_degen = true;
	  if (!opt_ec)
	    opt_ec = 1;
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
      else if (!strcmp(argv[argn], "-m"))
	{
	  opt_minim = true;
	  opt_replay = true;
	  if (!opt_ec)
	    opt_ec = 1;
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
      else if (!strcmp(argv[argn], "-z"))
	{
	  opt_z = true;
	}
      else if (!strcmp(argv[argn], "-Z"))
	{
	  opt_Z = opt_z = true;
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
	  spot::tgba* degen = 0;
	  if (opt_degen && opt_n_acc != 1)
	    degen = new spot::tgba_tba_proxy(a);

	  std::vector<spot::emptiness_check*> ec_obj;
	  std::vector<std::string> ec_name;
	  std::vector<bool> ec_safe;

	  ec_obj.push_back(new spot::couvreur99_check(a));
	  ec_name.push_back("couvreur99");
	  ec_safe.push_back(true);

	  ec_obj.push_back(new spot::couvreur99_check_shy(a));
	  ec_name.push_back("couvreur99_shy");
	  ec_safe.push_back(true);

	  if (opt_n_acc <= 1 || opt_degen)
	    {
	      spot::tgba* d = opt_n_acc > 1 ? degen : a;

	      ec_obj.push_back(spot::explicit_magic_search(d));
	      ec_name.push_back("explicit_magic_search");
	      ec_safe.push_back(true);

	      ec_obj.push_back(spot::bit_state_hashing_magic_search(d, 4096));
	      ec_name.push_back("bit_state_hashing_magic_search");
	      ec_safe.push_back(false);

	      ec_obj.push_back(spot::explicit_se05_search(d));
	      ec_name.push_back("explicit_se05");
	      ec_safe.push_back(true);

	      ec_obj.push_back(spot::bit_state_hashing_se05_search(d, 4096));
	      ec_name.push_back("bit_state_hashing_se05");
	      ec_safe.push_back(false);

	      ec_obj.push_back(spot::explicit_gv04_check(d));
	      ec_name.push_back("explicit_gv04");
	      ec_safe.push_back(true);
	    }

	  if (opt_n_acc >= 1 || opt_degen)
	    {
	      spot::tgba* d = opt_n_acc == 0 ? degen : a;

              ec_obj.push_back(spot::explicit_tau03_search(d));
              ec_name.push_back("explicit_tau03");
              ec_safe.push_back(true);
            }

          ec_obj.push_back(spot::explicit_tau03_opt_search(a));
          ec_name.push_back("explicit_tau03_opt");
          ec_safe.push_back(true);

	  int n_ec = ec_obj.size();
	  int n_empty = 0;
	  int n_non_empty = 0;
	  int n_maybe_empty = 0;

	  for (int i = 0; i < n_ec; ++i)
	    {
	      std::string algo = ec_name[i];
	      std::cout.width(32);
	      std::cout << algo << ": ";
	      spot::emptiness_check_result* res = ec_obj[i]->check();
	      const spot::ec_statistics* ecs =
                       dynamic_cast<const spot::ec_statistics*>(ec_obj[i]);
              if (opt_z && ecs)
		ec_stats[algo].count(ecs);
	      if (res)
		{
		  std::cout << "acc. run";
		  ++n_non_empty;
		  if (opt_replay)
		    {
		      spot::tgba_run* run = res->accepting_run();
		      if (run)
			{
			  std::ostringstream s;
			  if (!spot::replay_tgba_run(s, res->automaton(), run))
			    {
			      std::cout << ", but could not replay it (ERROR!)";
			      failed_seeds.insert(opt_ec_seed);
			    }
			  else
			    {
			      std::cout << ", computed";
			      if (opt_z)
				ar_stats[algo].count(run);
			    }
			  if (opt_z)
			    std::cout << " [" << run->prefix.size()
				      << "+" << run->cycle.size()
				      << "]";

			  if (opt_minim)
			    {
			      spot::tgba_run* minrun =
				spot::minimize_run(res->automaton(), run);
			      if (!spot::replay_tgba_run(s, res->automaton(),
							 minrun))
				{
				  std::cout << ", but could not replay "
					    << "its minimization (ERROR!)";
				  failed_seeds.insert(opt_ec_seed);
				}
			      else
				{
				  std::cout << ", minimized";
				  if (opt_z)
				    mar_stats[algo].count(minrun);
				}
			      if (opt_z)
				{
				  std::cout << " [" << minrun->prefix.size()
					    << "+" << minrun->cycle.size()
					    << "]";
				}
			      delete minrun;
			    }
			  delete run;
			}
		      else
			{
			  std::cout << " exists, not computed";
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

	      if (opt_Z)
		ec_obj[i]->print_stats(std::cout);
	      delete ec_obj[i];
	    }

	  assert(n_empty + n_non_empty + n_maybe_empty == n_ec);

	  if ((n_empty == 0 && (n_non_empty + n_maybe_empty) != n_ec)
	      || (n_empty != 0 && n_non_empty != 0))
	    {
	      std::cout << "ERROR: not all algorithms agree" << std::endl;
	      failed_seeds.insert(opt_ec_seed);
	    }

	  delete degen;
	}

      delete a;

      if (opt_ec)
	{
	  --opt_ec;
	  ++opt_ec_seed;
	}
    }
  while (opt_ec);

  if (!ec_stats.empty())
    {
      std::cout << "Statistics about emptiness checkers:" << std::endl;
      std::cout << std::setw(32) << ""
		<< " |       states      |    transitions    |"
		<< std::endl << std::setw(32) << "algorithm"
		<< " |  min < mean < max |  min < mean < max |   n "
		<< std::endl
		<< std::setw(79) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl << std::setprecision(3);
      for (ec_stats_type::const_iterator i = ec_stats.begin();
                                                i != ec_stats.end(); ++i)
	std::cout << std::setw(32) << i->first << " |"
		  << std::setw(5) << i->second.min_states
		  << " "
		  << std::setw(6)
		  << static_cast<float>(i->second.tot_states) / i->second.n
		  << " "
		  << std::setw(5) << i->second.max_states
		  << " |"
		  << std::setw(5) << i->second.min_transitions
		  << " "
		  << std::setw(6)
		  << static_cast<float>(i->second.tot_transitions) / i->second.n
		  << " "
		  << std::setw(5) << i->second.max_transitions
		  << " |"
		  << std::setw(5) << i->second.n
		  << std::endl;
      std::cout << std::setw(79) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl
		<< std::setw(32) << ""
		<< " |   maximal depth   |"
		<< std::endl << std::setw(32) << "algorithm"
		<< " |  min < mean < max |   n "
		<< std::endl
		<< std::setw(59) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl;
      for (ec_stats_type::const_iterator i = ec_stats.begin();
                                                i != ec_stats.end(); ++i)
	std::cout << std::setw(32) << i->first << " |"
		  << std::setw(5)
		  << i->second.min_max_depth
		  << " "
		  << std::setw(6)
		  << static_cast<float>(i->second.tot_max_depth) / i->second.n
		  << " "
		  << std::setw(5)
		  << i->second.max_max_depth
		  << " |"
		  << std::setw(5) << i->second.n
		  << std::endl;
    }

  if (!ar_stats.empty())
    {
      std::cout << std::endl
		<< "Statistics about accepting runs:" << std::endl;
      print_ar_stats(ar_stats);
    }
  if (!mar_stats.empty())
    {
      std::cout << std::endl
		<< "Statistics about minimized accepting runs:" << std::endl;
      print_ar_stats(mar_stats);
    }

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
