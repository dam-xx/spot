// Copyright (C) 2004, 2005  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <utility>
#include <set>
#include <string>
#include "ltlparse/public.hh"
#include "ltlvisit/apcollect.hh"
#include "ltlvisit/destroy.hh"
#include "ltlvisit/randomltl.hh"
#include "ltlvisit/tostring.hh"
#include "ltlvisit/length.hh"
#include "ltlvisit/reduce.hh"
#include "tgbaalgos/randomgraph.hh"
#include "tgbaalgos/save.hh"
#include "tgbaalgos/stats.hh"
#include "ltlenv/defaultenv.hh"
#include "ltlast/atomic_prop.hh"
#include "tgbaalgos/dotty.hh"
#include "tgbaparse/public.hh"
#include "misc/random.hh"
#include "tgba/tgbatba.hh"
#include "tgba/tgbaproduct.hh"
#include "misc/timer.hh"

#include "tgbaalgos/ltl2tgba_fm.hh"

#include "tgbaalgos/emptiness.hh"
#include "tgbaalgos/emptiness_stats.hh"
#include "tgbaalgos/gtec/gtec.hh"
#include "tgbaalgos/gv04.hh"
#include "tgbaalgos/magic.hh"
#include "tgbaalgos/reducerun.hh"
#include "tgbaalgos/se05.hh"
#include "tgbaalgos/tau03.hh"
#include "tgbaalgos/tau03opt.hh"
#include "tgbaalgos/replayrun.hh"


spot::emptiness_check*
couvreur99_cons(const spot::tgba* a)
{
  return new spot::couvreur99_check(a);
}

spot::emptiness_check*
couvreur99_shy_cons(const spot::tgba* a)
{
  return new spot::couvreur99_check_shy(a);
}

spot::emptiness_check*
couvreur99_shy_minus_cons(const spot::tgba* a)
{
  return new spot::couvreur99_check_shy(a, false);
}

spot::emptiness_check*
bsh_ms_cons(const spot::tgba* a)
{
  return spot::bit_state_hashing_magic_search(a, 4096);
}

spot::emptiness_check*
bsh_se05_cons(const spot::tgba* a)
{
  return spot::bit_state_hashing_se05_search(a, 4096);
}

struct ec_algo
{
  const char* name;
  spot::emptiness_check* (*construct)(const spot::tgba*);
  unsigned int min_acc;
  unsigned int max_acc;
  bool safe;
};
ec_algo ec_algos[] =
  {
    { "couvreur99",            couvreur99_cons,                 0, -1U, true },
    { "couvreur99_shy-",       couvreur99_shy_minus_cons,       0, -1U, true },
    { "couvreur99_shy",        couvreur99_shy_cons,             0, -1U, true },
    { "explicit_magic_search", spot::explicit_magic_search,     0,   1, true },
    { "bsh_magic_search",      bsh_ms_cons,                     0,   1, false },
    { "explicit_se05",         spot::explicit_se05_search,      0,   1, true },
    { "bsh_se05",              bsh_se05_cons,                   0,   1, false },
    { "explicit_gv04",         spot::explicit_gv04_check,       0,   1, true },
    { "explicit_tau03",        spot::explicit_tau03_search,     1, -1U, true },
    { "explicit_tau03_opt",    spot::explicit_tau03_opt_search, 0, -1U, true },
  };

spot::emptiness_check*
cons_emptiness_check(int num, const spot::tgba* a,
		     const spot::tgba* degen, unsigned int n_acc)
{
  if (n_acc < ec_algos[num].min_acc || n_acc > ec_algos[num].max_acc)
    a = degen;
  if (a)
    return ec_algos[num].construct(a);
  return 0;
}

void
syntax(char* prog)
{
  std::cerr << "Usage: "<< prog << " [OPTIONS...] PROPS..." << std::endl
	    << std::endl
	    << "Options:" << std::endl
	    << "  -0      suppress default output, just generate the graph"
	    << " in memory" << std::endl
	    << "  -a N F  number of acceptance conditions and probability that"
	    << " one is true" << std::endl
	    << "            [0 0.0]" << std::endl
	    << "  -d F    density of the graph [0.2]" << std::endl
	    << "  -dp     dump priorities, do not generate any formula"
	    << std::endl
	    << "  -D      degeneralize TGBA for emptiness-check algorithms that"
	    << " would" << std::endl
	    << "            otherwise be skipped (implies -e)" << std::endl
	    << "  -e N    compare result of all "
	    << "emptiness checks on N randomly generated graphs" << std::endl
	    << "  -f N    the size of the formula [15]" << std::endl
	    << "  -F N    number of formulae to generate [0]" << std::endl
	    << "  -g      output in dot format" << std::endl
	    << "  -i FILE do not generate formulae, read them from FILE"
            << std::endl
	    << "  -l N    simplify formulae using all available reductions"
	    << " and reject those" << std::endl
	    << "            strictly smaller than N" << std::endl
	    << "  -m      try to reduce runs, in a second pass (implies -r)"
	    << std::endl
	    << "  -n N    number of nodes of the graph [20]" << std::endl
	    << "  -O ALGO run Only ALGO" << std::endl
	    << "  -p S    priorities to use" << std::endl
	    << "  -r      compute and replay acceptance runs (implies -e)"
	    << std::endl
	    << "  -R N    repeat each emptiness-check and accepting run "
	    << "computation N times" << std::endl
	    << "  -s N    seed for the random number generator" << std::endl
	    << "  -t F    probability of the atomic propositions to be true"
	    << " [0.5]" << std::endl
	    << "  -u      generate unique formulae"
	    << std::endl
	    << "  -z      display statistics about computed accepting runs"
	    << std::endl
	    << "  -Z      like -z, but print extra statistics after the run"
	    << " of each algorithm" << std::endl
	    << "Where:" << std::endl
	    << "  F      are floats between 0.0 and 1.0 inclusive" << std::endl
	    << "  E      are floating values" << std::endl
	    << "  S      are `KEY=E, KEY=E, ...' strings" << std::endl
	    << "  N      are positive integers" << std::endl
	    << "  PROPS  are the atomic properties to use on transitions"
	    << std::endl
	    << "Use -dp to see the list of KEYs." << std::endl;
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
  int max_tgba_states;
  int tot_tgba_states;
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

struct ratio_stat
{
  float min_ratio_states;
  float max_ratio_states;
  float tot_ratio_states;
  float min_ratio_transitions;
  float max_ratio_transitions;
  float tot_ratio_transitions;
  float min_ratio_depth;
  float max_ratio_depth;
  float tot_ratio_depth;
  int n;

  ratio_stat()
    : n(0)
  {
  }

  void
  count(const spot::ec_statistics* ec, const spot::tgba* a)
  {
    spot::tgba_statistics a_size = spot::stats_reachable(a);
    float ms = ec->states() / static_cast<float>(a_size.states);
    float mt = ec->transitions() / static_cast<float>(a_size.transitions);
    float mm = ec->max_depth() / static_cast<float>(a_size.states);

    if (n++)
      {
        min_ratio_states = std::min(min_ratio_states, ms);
        max_ratio_states = std::max(max_ratio_states, ms);
        tot_ratio_states += ms;
        min_ratio_transitions = std::min(min_ratio_transitions, mt);
        max_ratio_transitions = std::max(max_ratio_transitions, mt);
        tot_ratio_transitions += mt;
        min_ratio_depth = std::min(min_ratio_depth, mm);
        max_ratio_depth = std::max(max_ratio_depth, mm);
        tot_ratio_depth += mm;
      }
    else
      {
        min_ratio_states = max_ratio_states = tot_ratio_states = ms;
        min_ratio_transitions = max_ratio_transitions =
                                                    tot_ratio_transitions = mt;
        min_ratio_depth = max_ratio_depth = tot_ratio_depth = mm;
      }
  }
};

struct acss_stat
{
  int min_states;
  int max_states;
  int tot_states;
  int n;

  acss_stat()
    : n(0)
  {
  }

  void
  count(const spot::acss_statistics* acss)
  {
    int s = acss->acss_states();
    if (n++)
      {
	min_states = std::min(min_states, s);
	max_states = std::max(max_states, s);
	tot_states += s;
      }
    else
      {
	min_states = max_states = tot_states = s;
      }
  }
};

struct ars_stat
{
  int min_states;
  int max_states;
  int tot_states;
  int n;

  ars_stat()
    : n(0)
  {
  }

  void
  count(const spot::ars_statistics* acss)
  {
    int s = acss->ars_states();
    if (n++)
      {
	min_states = std::min(min_states, s);
	max_states = std::max(max_states, s);
	tot_states += s;
      }
    else
      {
	min_states = max_states = tot_states = s;
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

typedef std::map<std::string, ratio_stat> ratio_stat_type;
ratio_stat_type glob_ratio_stats;
typedef std::map<int, ratio_stat_type > ratio_stats_type;
ratio_stats_type ratio_stats;

typedef std::map<std::string, acss_stat> acss_stats_type;
acss_stats_type acss_stats;

typedef std::map<std::string, ars_stat> ars_stats_type;
ars_stats_type ars_stats;

typedef std::map<std::string, ar_stat> ar_stats_type;
ar_stats_type ar_stats;		// Statistics about accepting runs.
ar_stats_type mar_stats;        // ... about minimized accepting runs.

void
print_ar_stats(ar_stats_type& ar_stats)
{
  std::ios::fmtflags old = std::cout.flags();
  std::cout << std::right << std::fixed << std::setprecision(1);

  std::cout << std::setw(22) << ""
            << " |         prefix        |         cycle         |"
            << std::endl
            << std::setw(22) << "algorithm"
            << " |   min   < mean  < max |   min   < mean  < max |   n"
            << std::endl
            << std::setw(79) << std::setfill('-') << "" << std::setfill(' ')
            << std::endl;
  for (ar_stats_type::const_iterator i = ar_stats.begin();
        i != ar_stats.end(); ++i)
    std::cout << std::setw(22) << i->first << " |"
              << std::setw(6) << i->second.min_prefix
              << " "
              << std::setw(8)
              << static_cast<float>(i->second.tot_prefix) / i->second.n
              << " "
              << std::setw(6) << i->second.max_prefix
              << " |"
              << std::setw(6) << i->second.min_cycle
              << " "
              << std::setw(8)
              << static_cast<float>(i->second.tot_cycle) / i->second.n
              << " "
              << std::setw(6) << i->second.max_cycle
              << " |"
              << std::setw(4) << i->second.n
              << std::endl;
  std::cout << std::setw(79) << std::setfill('-') << "" << std::setfill(' ')
            << std::endl
            << std::setw(22) << ""
            << " |          runs         |         total         |"
            << std::endl <<
            std::setw(22) << "algorithm"
            << " |   min   < mean  < max |  pre.   cyc.     runs |   n"
            << std::endl
            << std::setw(79) << std::setfill('-') << "" << std::setfill(' ')
            << std::endl;
  for (ar_stats_type::const_iterator i = ar_stats.begin();
        i != ar_stats.end(); ++i)
    std::cout << std::setw(22) << i->first << " |"
              << std::setw(6)
              << i->second.min_run
              << " "
              << std::setw(8)
              << static_cast<float>(i->second.tot_prefix
                                    + i->second.tot_cycle) / i->second.n
              << " "
              << std::setw(6)
              << i->second.max_run
              << " |"
              << std::setw(6) << i->second.tot_prefix
              << " "
              << std::setw(6) << i->second.tot_cycle
              << " "
              << std::setw(8) << i->second.tot_prefix + i->second.tot_cycle
              << " |"
              << std::setw(4) << i->second.n
              << std::endl;

  std::cout << std::setiosflags(old);
}

void
print_ratio_stats(int nbac, const ratio_stat_type& ratio_stats)
{
      std::cout << std::endl;
      std::ios::fmtflags old = std::cout.flags();
      std::cout << std::right << std::fixed << std::setprecision(1);

      std::cout << "Ratios about emptiness checkers: ";
      if (nbac >= 0)
        std::cout << "(" << nbac << " acceptance conditions)";
      std::cout << std::endl;
      std::cout << std::setw(22) << ""
		<< " |       % states        |    % transitions      |"
		<< std::endl << std::setw(22) << "algorithm"
		<< " |   min   < mean  < max |   min   < mean  < max |   n"
		<< std::endl
		<< std::setw(79) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl;
      for (ratio_stat_type::const_iterator i = ratio_stats.begin();
	   i != ratio_stats.end(); ++i)
	std::cout << std::setw(22) << i->first << " |"
		  << std::setw(6) << i->second.min_ratio_states * 100.
		  << " "
		  << std::setw(8)
		  << (static_cast<float>(i->second.tot_ratio_states) /
                                                            i->second.n) * 100.
		  << " "
		  << std::setw(6) << i->second.max_ratio_states * 100.
		  << " |"
		  << std::setw(6) << i->second.min_ratio_transitions * 100.
		  << " "
		  << std::setw(8)
		  << (static_cast<float>(i->second.tot_ratio_transitions) /
                                                            i->second.n) * 100.
		  << " "
		  << std::setw(6) << i->second.max_ratio_transitions * 100.
		  << " |"
		  << std::setw(4) << i->second.n
		  << std::endl;
      std::cout << std::setw(79) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl
		<< std::setw(22) << ""
		<< " |   % maximal depth     |"
		<< std::endl
                << std::setw(22) << "algorithm"
		<< " |   min   < mean  < max |   n"
		<< std::endl
		<< std::setw(53) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl;
      for (ratio_stat_type::const_iterator i = ratio_stats.begin();
	   i != ratio_stats.end(); ++i)
	std::cout << std::setw(22) << i->first << " |"
		  << std::setw(6)
		  << i->second.min_ratio_depth * 100.
		  << " "
		  << std::setw(8)
		  << (static_cast<float>(i->second.tot_ratio_depth) /
                                                          i->second.n) * 100.
		  << " "
		  << std::setw(6)
		  << i->second.max_ratio_depth * 100.
		  << " |"
		  << std::setw(4) << i->second.n
		  << std::endl;

      std::cout << std::setiosflags(old);
}

spot::ltl::formula*
generate_formula(const spot::ltl::random_ltl& rl, int opt_f, int opt_s,
                 int opt_l = 0, bool opt_u = false)
{
  static std::set<std::string> unique;

  int max_tries_u = 1000;
  while (max_tries_u--)
    {
      spot::srand(opt_s++);
      spot::ltl::formula* f;
      int max_tries_l = 1000;
      while (max_tries_l--)
        {
          f = rl.generate(opt_f);
          if (opt_l)
            {
              spot::ltl::formula* g = reduce(f);
              spot::ltl::destroy(f);
              if (spot::ltl::length(g) < opt_l)
                {
                  spot::ltl::destroy(g);
                  continue;
                }
              f = g;
            }
          else
            {
              assert(spot::ltl::length(f) <= opt_f);
            }
          break;
        }
      if (max_tries_l < 0)
        {
          assert(opt_l);
          std::cerr << "Failed to generate non-reducible formula "
                    << "of size " << opt_l << " or more." << std::endl;
          return 0;
        }
      std::string txt = spot::ltl::to_string(f);
      if (!opt_u || unique.insert(txt).second)
        {
          return f;
        }
      spot::ltl::destroy(f);
    }
  assert(opt_u);
  std::cerr << "Failed to generate another unique formula."
            << std::endl;
  return 0;
}

int
main(int argc, char** argv)
{
  bool opt_dp = false;
  int opt_f = 15;
  int opt_F = 0;
  char* opt_p = 0;
  char* opt_i = 0;
  std::istream *formula_file = 0;
  int opt_l = 0;
  bool opt_u = false;

  int opt_n_acc = 0;
  float opt_a = 0.0;
  float opt_d = 0.2;
  int opt_n = 20;
  float opt_t = 0.5;

  bool opt_0 = false;
  bool opt_z = false;
  bool opt_Z = false;

  int opt_R = 0;

  const char* opt_O = 0;

  bool opt_dot = false;
  int opt_ec = 0;
  int opt_ec_seed = 0;
  bool opt_reduce = false;
  bool opt_replay = false;
  bool opt_degen = false;
  int argn = 0;

  int exit_code = 0;

  spot::tgba* formula = 0;
  spot::tgba* product = 0;

  spot::ltl::environment& env(spot::ltl::default_environment::instance());
  spot::ltl::atomic_prop_set* ap = new spot::ltl::atomic_prop_set;
  spot::bdd_dict* dict = new spot::bdd_dict();

  if (argc <= 1)
    syntax(argv[0]);

  while (++argn < argc)
    {
      if (!strcmp(argv[argn], "-0"))
	{
	  opt_0 = true;
	}
      else if (!strcmp(argv[argn], "-a"))
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
	  opt_reduce = true;
	  opt_replay = true;
	  if (!opt_ec)
	    opt_ec = 1;
	}
      else if (!strcmp(argv[argn], "-i"))
        {
          if (argc < argn + 2)
            syntax(argv[0]);
          opt_i = argv[++argn];
          if (strcmp(opt_i, "-"))
            {
              formula_file = new std::ifstream(opt_i);
              if (!(*formula_file))
                {
                  delete formula_file;
                  std::cerr << "Failed to open " << opt_i << std::endl;
                  exit(2);
                }
            }
          else
            formula_file = &std::cin;
        }
      else if (!strcmp(argv[argn], "-n"))
	{
	  if (argc < argn + 2)
	    syntax(argv[0]);
	  opt_n = to_int(argv[++argn]);
	}
      else if (!strcmp(argv[argn], "-O"))
	{
	  if (argc < argn + 2)
	    syntax(argv[0]);
	  opt_O = argv[++argn];
	  int s = sizeof(ec_algos) / sizeof(*ec_algos);
	  int i;
	  for (i = 0; i < s; ++i)
	    if (!strcmp(opt_O, ec_algos[i].name))
	      break;
	  if (i == s)
	    {
	      std::cerr << "Unknown algorithm.  Available algorithms are:"
			<< std::endl;
	      for (i = 0; i < s; ++i)
		std::cerr << "  " << ec_algos[i].name << std::endl;
	      exit(1);
	    }
	}
      else if (!strcmp(argv[argn], "-r"))
	{
	  opt_replay = true;
	  if (!opt_ec)
	    opt_ec = 1;
	}
      else if (!strcmp(argv[argn], "-R"))
	{
	  if (argc < argn + 2)
	    syntax(argv[0]);
	  opt_R = to_int(argv[++argn]);
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
      else if (!strcmp(argv[argn], "-dp"))
	{
	  opt_dp = true;
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
      else if (!strcmp(argv[argn], "-l"))
	{
	  if (argc < argn + 2)
	    syntax(argv[0]);
	  opt_l = to_int(argv[++argn]);
	}
      else if (!strcmp(argv[argn], "-u"))
	{
	  opt_u = true;
	}
      else
	{
	  ap->insert(static_cast<spot::ltl::atomic_prop*>
		     (env.require(argv[argn])));
	}
    }

  spot::ltl::random_ltl rl(ap);
  const char* tok = rl.parse_options(opt_p);
  if (tok)
    {
      std::cerr << "failed to parse probabilities near `"
		<< tok << "'" << std::endl;
      exit(2);
    }

  if (opt_l > opt_f)
    {
      std::cerr << "-l's argument (" << opt_l << ") should not be larger than "
		<< "-f's (" << opt_f << ")" << std::endl;
      exit(2);
    }

  if (opt_dp)
    {
      rl.dump_priorities(std::cout);
      exit(0);
    }

  spot::timer_map tm_ec;
  spot::timer_map tm_ar;
  std::set<int> failed_seeds;
  int init_opt_ec = opt_ec;
  spot::ltl::atomic_prop_set* apf = new spot::ltl::atomic_prop_set;

  do
    {
      if (opt_F)
        {
          spot::ltl::formula* f = generate_formula(rl, opt_f, opt_ec_seed,
                                                  opt_l, opt_u);
          if (!f)
            exit(1);
          formula = spot::ltl_to_tgba_fm(f, dict, true);
          spot::ltl::destroy(f);
        }
      else if (opt_i)
        {
          if (formula_file->good())
            {
              std::string input;
              std::getline(*formula_file, input, '\n');
              spot::ltl::parse_error_list pel;
              spot::ltl::formula* f = spot::ltl::parse(input, pel, env);
              if (spot::ltl::format_parse_errors(std::cerr, input, pel))
                break;
              formula = spot::ltl_to_tgba_fm(f, dict, true);
              spot::ltl::atomic_prop_set* tmp =
                                            spot::ltl::atomic_prop_collect(f);
              for (spot::ltl::atomic_prop_set::iterator i = tmp->begin();
                                                        i != tmp->end(); ++i)
                  apf->insert(
                            dynamic_cast<spot::ltl::atomic_prop*>((*i)->ref()));
              spot::ltl::destroy(f);
              delete tmp;
            }
          else
            {
              if (formula_file->bad())
                std::cerr << "Failed to read " << opt_i << std::endl;
              else
                std::cerr << "End of " << opt_i << std::endl;
              break;
            }
        }

     for (spot::ltl::atomic_prop_set::iterator i = ap->begin();
                                                         i != ap->end(); ++i)
         apf->insert(dynamic_cast<spot::ltl::atomic_prop*>((*i)->ref()));

      do
        {
          if (opt_ec)
            {
              std::cout << "seed: " << opt_ec_seed << std::endl;
              spot::srand(opt_ec_seed);
            }

          spot::tgba* a;
          spot::tgba* r = a = spot::random_graph(opt_n, opt_d, apf, dict,
                                                opt_n_acc, opt_a, opt_t, &env);

          if (formula)
            a = product = new spot::tgba_product(formula, a);

          int real_n_acc = a->number_of_acceptance_conditions();

          if (!opt_ec)
            {
              if (opt_dot)
                dotty_reachable(std::cout, a);
              else if (!opt_0)
                tgba_save_reachable(std::cout, a);
            }
          else
            {
              spot::tgba* degen = 0;
              if (opt_degen && real_n_acc != 1)
                degen = new spot::tgba_tba_proxy(a);

              int n_alg = sizeof(ec_algos) / sizeof(*ec_algos);
              int n_ec = 0;
              int n_empty = 0;
              int n_non_empty = 0;
              int n_maybe_empty = 0;

              for (int i = 0; i < n_alg; ++i)
                {
                  spot::emptiness_check* ec;
                  spot::emptiness_check_result* res;
                  if (opt_O && strcmp(opt_O, ec_algos[i].name))
                    continue;
                  ec = cons_emptiness_check(i, a, degen, real_n_acc);
                  if (!ec)
                    continue;
                  ++n_ec;
                  std::string algo = ec_algos[i].name;
                  std::cout.width(32);
                  std::cout << algo << ": ";
                  tm_ec.start(algo);
                  for (int count = opt_R;;)
                    {
                      res = ec->check();
                      if (count-- <= 0)
                        break;
                      delete res;
                      delete ec;
                      ec = cons_emptiness_check(i, a, degen, real_n_acc);
                    }
                  tm_ec.stop(algo);
                  const spot::ec_statistics* ecs =
                    dynamic_cast<const spot::ec_statistics*>(ec);
                  if (opt_z && ecs)
                    {
                      ec_stats[algo].count(ecs);
                      if (res)
                        {
                          // Notice that ratios are computed w.r.t. the
                          // generalized automaton a.
                          int nba = a->number_of_acceptance_conditions();
                          ratio_stats[nba][algo].count(ecs, a);
                          glob_ratio_stats[algo].count(ecs, a);
                        }
                    }
                  if (res)
                    {
                      std::cout << "acc. run";
                      ++n_non_empty;
                      const spot::acss_statistics* acss =
                        dynamic_cast<const spot::acss_statistics*>(res);
                      if (opt_z && acss)
                        acss_stats[algo].count(acss);
                      if (opt_replay)
                        {
                          spot::tgba_run* run;

                          const spot::ars_statistics* ars =
                            dynamic_cast<const spot::ars_statistics*>(res);

                          tm_ar.start(algo);
                          for (int count = opt_R;;)
                            {
                              run = res->accepting_run();
                              if (opt_z && ars)
                                {
                                  // Count only the first run (the other way
                                  // would be to divide the stats by opt_R).
                                  ars_stats[algo].count(ars);
                                  ars = 0;
                                }
                              if (count-- <= 0 || !run)
                                break;
                              delete run;
                            }
                          if (!run)
                            {
                              tm_ar.cancel(algo);
                              std::cout << " exists, not computed";
                            }
                          else
                            {
                              tm_ar.stop(algo);
                              std::ostringstream s;
                              if (!spot::replay_tgba_run(s, res->automaton(),
                                                        run))
                                {
                                  std::cout << ", but could not replay it "
                                            << "(ERROR!)";
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

                              if (opt_reduce)
                                {
                                  spot::tgba_run* redrun =
                                    spot::reduce_run(res->automaton(), run);
                                  if (!spot::replay_tgba_run(s,
                                                    res->automaton(), redrun))
                                    {
                                      std::cout << ", but could not replay "
                                                << "its minimization (ERROR!)";
                                      failed_seeds.insert(opt_ec_seed);
                                    }
                                  else
                                    {
                                      std::cout << ", reduced";
                                      if (opt_z)
                                        mar_stats[algo].count(redrun);
                                    }
                                  if (opt_z)
                                    {
                                      std::cout << " [" << redrun->prefix.size()
                                                << "+" << redrun->cycle.size()
                                                << "]";
                                    }
                                  delete redrun;
                                }
                              delete run;
                            }
                        }
                      std::cout << std::endl;
                      delete res;
                    }
                  else
                    {
                      if (ec_algos[i].safe)
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
                    ec->print_stats(std::cout);
                  delete ec;
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

          delete product;
          delete r;

          if (opt_ec)
            {
              --opt_ec;
              ++opt_ec_seed;
            }
        }
      while (opt_ec);

      delete formula;
      if (opt_F)
        --opt_F;
      opt_ec = init_opt_ec;
      for (spot::ltl::atomic_prop_set::iterator i = apf->begin();
          i != apf->end(); ++i)
        spot::ltl::destroy(*i);
      apf->clear();
    }
  while (opt_F || opt_i);

  if (!ec_stats.empty())
    {
      std::cout << std::endl;
      std::ios::fmtflags old = std::cout.flags();
      std::cout << std::right << std::fixed << std::setprecision(1);

      std::cout << "Statistics about emptiness checkers:" << std::endl;
      std::cout << std::setw(22) << ""
		<< " |         states        |      transitions      |"
		<< std::endl << std::setw(22) << "algorithm"
		<< " |   min   < mean  < max |   min   < mean  < max |   n"
		<< std::endl
		<< std::setw(79) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl;
      for (ec_stats_type::const_iterator i = ec_stats.begin();
	   i != ec_stats.end(); ++i)
	std::cout << std::setw(22) << i->first << " |"
		  << std::setw(6) << i->second.min_states
		  << " "
		  << std::setw(8)
		  << static_cast<float>(i->second.tot_states) / i->second.n
		  << " "
		  << std::setw(6) << i->second.max_states
		  << " |"
		  << std::setw(6) << i->second.min_transitions
		  << " "
		  << std::setw(8)
		  << static_cast<float>(i->second.tot_transitions) / i->second.n
		  << " "
		  << std::setw(6) << i->second.max_transitions
		  << " |"
		  << std::setw(4) << i->second.n
		  << std::endl;
      std::cout << std::setw(79) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl
		<< std::setw(22) << ""
		<< " |     maximal depth     |"
		<< std::endl
                << std::setw(22) << "algorithm"
		<< " |   min   < mean  < max |   n"
		<< std::endl
		<< std::setw(53) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl;
      for (ec_stats_type::const_iterator i = ec_stats.begin();
	   i != ec_stats.end(); ++i)
	std::cout << std::setw(22) << i->first << " |"
		  << std::setw(6)
		  << i->second.min_max_depth
		  << " "
		  << std::setw(8)
		  << static_cast<float>(i->second.tot_max_depth) / i->second.n
		  << " "
		  << std::setw(6)
		  << i->second.max_max_depth
		  << " |"
		  << std::setw(4) << i->second.n
		  << std::endl;

      std::cout << std::setiosflags(old);
    }

  if (!glob_ratio_stats.empty())
    {
      print_ratio_stats(-1, glob_ratio_stats);
      for (ratio_stats_type::const_iterator i = ratio_stats.begin();
           i != ratio_stats.end(); ++i)
        print_ratio_stats(i->first, i->second);
    }

  if (!acss_stats.empty())
    {
      std::cout << std::endl;
      std::ios::fmtflags old = std::cout.flags();
      std::cout << std::right << std::fixed << std::setprecision(1);

      std::cout << "Statistics about accepting cycle search space:"
		<< std::endl;
      std::cout << std::setw(22) << ""
		<< " |              states           |"
		<< std::endl << std::setw(22) << "algorithm"
		<< " |   min   < mean  < max | total |  n"
		<< std::endl
		<< std::setw(61) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl;
      for (acss_stats_type::const_iterator i = acss_stats.begin();
	   i != acss_stats.end(); ++i)
	std::cout << std::setw(22) << i->first << " |"
		  << std::setw(6) << i->second.min_states
		  << " "
		  << std::setw(8)
		  << static_cast<float>(i->second.tot_states) / i->second.n
		  << " "
		  << std::setw(6) << i->second.max_states
		  << " |"
		  << std::setw(6) << i->second.tot_states
		  << " |"
		  << std::setw(4) << i->second.n
		  << std::endl;
      std::cout << std::setiosflags(old);
    }
  if (!ars_stats.empty())
    {
      std::cout << std::endl;
      std::ios::fmtflags old = std::cout.flags();
      std::cout << std::right << std::fixed << std::setprecision(1);

      std::cout << "Statistics about accepting run computation:"
		<< std::endl;
      std::cout << std::setw(22) << ""
		<< " |      (non unique) states      |"
		<< std::endl << std::setw(22) << "algorithm"
		<< " |   min   < mean  < max | total |  n"
		<< std::endl
		<< std::setw(61) << std::setfill('-') << "" << std::setfill(' ')
		<< std::endl;
      for (ars_stats_type::const_iterator i = ars_stats.begin();
	   i != ars_stats.end(); ++i)
	std::cout << std::setw(22) << i->first << " |"
		  << std::setw(6) << i->second.min_states
		  << " "
		  << std::setw(8)
		  << static_cast<float>(i->second.tot_states) / i->second.n
		  << " "
		  << std::setw(6) << i->second.max_states
		  << " |"
		  << std::setw(6) << i->second.tot_states
		  << " |"
		  << std::setw(4) << i->second.n
		  << std::endl;
      std::cout << std::setiosflags(old);
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
		<< "Statistics about reduced accepting runs:" << std::endl;
      print_ar_stats(mar_stats);
    }

  if (opt_z)
    {
      if (!tm_ec.empty())
	{
	  std::cout << std::endl
		    << "emptiness checks cumulated timings:" << std::endl;
	  tm_ec.print(std::cout);
	}
      if (!tm_ar.empty())
	{
	  std::cout << std::endl
		    << "accepting runs cumulated timings:" << std::endl;
	  tm_ar.print(std::cout);
	}
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

  if (opt_i && strcmp(opt_i, "-"))
    {
      //formula_file->close();
      delete formula_file;
    }
  delete ap;
  delete apf;
  delete dict;
  return exit_code;
}
