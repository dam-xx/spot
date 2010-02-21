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

#include <iosfwd>
#include <set>
#include <list>
#include <string>
#include <queue>
#include <cstdlib>
#include <cstring>
#include <sys/time.h>
#include <sstream>
#include "ltlast/atomic_prop.hh"
#include "ltlast/unop.hh"
#include "ltlenv/defaultenv.hh"
#include "ltlparse/ltlfile.hh"
#include "ltlparse/public.hh"
#include "ltlvisit/apcollect.hh"
#include "ltlvisit/randomltl.hh"
#include "ltlvisit/tostring.hh"
#include "tgba/bdddict.hh"
#include "tgba/public.hh"
#include "tgba/tgbaproduct.hh"
#include "tgba/tgbaunion.hh"
#include "tgba/tgbasafracomplement.hh"
#include "tgbaalgos/dotty.hh"
#include "tgbaalgos/emptiness.hh"
#include "tgbaalgos/ltl2tgba_fm.hh"
#include "tgbaalgos/ltl2tgba_lacim.hh"
#include "tgbaalgos/randomgraph.hh"
#include "tgbaalgos/scc.hh"
#include "tgbaalgos/setdotdec.hh"
#include "tgbaalgos/cutscc.hh"
#include "misc/random.hh"

// Introduction: This is a program to test the ltl to tgba algorithms
// in Spot, 3 tests are computed as described in [TauHel02].

// In this file [TauHel02] corresponds to the following article:
// @article{TauHel02,
//   author = {Heikki Tauriainen and Keijo Heljanko},
//   journal = {International Journal on Software Tools for Technology
//                                                    Transfer (STTT)},
//   month = {October},
//   number = {1},
//   pages = {57--70},
//   publisher = {Springer-Verlag},
//   title = {Testing {LTL} formula translation into {B}\"uchi automata},
//   volume = {4},
//   year = {2002},
// }

// A pair of tgba corresponding to the automaton of the formula (first)
// and the automaton of the negated formula (second);
typedef std::pair<spot::tgba*, spot::tgba*> tgba_pair;
// A map associating the name of the algorithm to the generated tgbas
// with this algorithm.
typedef std::map<std::string, tgba_pair> tgba_map;

// For each ltl2tgba algorithm, we create a pair of automata.
tgba_map* build_tgba_map(const spot::ltl::formula* f,
                         spot::bdd_dict* dict,
                         std::list<std::string>& algos)
{
  spot::ltl::formula* f_neg;
  f_neg = spot::ltl::unop::instance(spot::ltl::unop::Not, f->clone());
  tgba_map* res = new tgba_map;

  // Function dispatch will be handled by ltl_translator_instantiator.
  std::list<std::string>::iterator it;
  for (it = algos.begin(); it != algos.end(); ++it)
  {
    if (*it == "fm")
    {
      spot::tgba* fm_prop = ltl_to_tgba_fm(f, dict);
      spot::tgba* fm_prop_neg = ltl_to_tgba_fm(f_neg, dict);
      tgba_pair fm_pair(fm_prop, fm_prop_neg);
      (*res)["fm"] = fm_pair;
    }
    else if (*it == "lacim")
    {
      spot::tgba* lacim_prop = ltl_to_tgba_lacim(f, dict);
      spot::tgba* lacim_prop_neg = ltl_to_tgba_lacim(f_neg, dict);
      tgba_pair lacim_pair(lacim_prop, lacim_prop_neg);
      (*res)["lacim"] = lacim_pair;
    }
    else if (*it == "taa")
    {
      // FIXME: when using TAA, there are no acceptance conditions on the
      // resulting TGBA.
      std::cerr << "Do not use TAA for now." << std::endl;
      exit(1);
      /*
      spot::tgba* taa_prop = ltl_to_taa(f, dict, false);
      spot::tgba* taa_prop_neg = ltl_to_taa(f_neg, dict, false);
      tgba_pair taa_pair(taa_prop, taa_prop_neg);
      (*res)["taa"] = taa_pair;
      */
    }
  }
  f_neg->destroy();
  return res;
}

// Free a tgba_map.
void free_tgba_map(tgba_map* m)
{
  tgba_map::iterator it;
  for (it = m->begin(); it != m->end(); ++it)
  {
    tgba_pair& p = it->second;
    delete p.first;
    delete p.second;
  }
  delete m;
}

// 'Emptiness check for the intersection of two Buchi automata' as described in
// [TauHel02].
// Given a LTL formula p and its negation !p, the assertion:
// L(p) inter L(!p) = empty_set must hold.
// To check the ltl to tgba translator, we compute the Buchi automata Ap and
// A!p.
// If L(Ap inter A!p) is not empty, then either Ap or A!p does not correspond
// to p or !p.
bool check_intersection(tgba_map* m,
                        spot::emptiness_check_instantiator* inst)
{
  bool res = true;
  tgba_map::iterator it1;
  tgba_map::iterator it2;
  for (it1 = m->begin(); it1 != m->end(); ++it1)
    for (it2 = m->begin(); it2 != m->end(); ++it2)
    {
      tgba_pair& p1 = it1->second;
      tgba_pair& p2 = it2->second;
      // Compute Ap inter A!p.
      spot::tgba_product* prod = new spot::tgba_product(p1.first, p2.second);
      spot::emptiness_check* ec = inst->instantiate(prod);
      spot::emptiness_check_result* acc = ec->check();
      bool is_empty = (acc == 0);
      delete ec;
      delete prod;
      delete acc;
      // If the intersection is not empty, one of the translator failed.
      if (!is_empty)
        return false;
    }
  return res;
}

// Compute the size of a TGBA (number of state), through a
// breadth-first traversal.
unsigned tgba_size(const spot::tgba* a)
{
  spot::state_set seen;
  std::queue<spot::state*> tovisit;
  // Perform breadth-first traversal.
  spot::state* init = a->get_init_state();
  tovisit.push(init);
  seen.insert(init);
  unsigned count = 0;
  // While there are still states to visit.
  while (!tovisit.empty())
  {
    ++count;
    spot::state* cur = tovisit.front();
    tovisit.pop();
    spot::tgba_succ_iterator* sit = a->succ_iter(cur);
    for (sit->first(); !sit->done(); sit->next())
    {
      spot::state* dst = sit->current_state();
      // Is it a new state ?
      if (seen.find(dst) == seen.end())
      {
        // Yes, register the successor for later processing.
        tovisit.push(dst);
        seen.insert(dst);
      }
      else
        // No, free dst.
        delete dst;
    }
    delete sit;
  }
  spot::state_set::iterator it2;
  // Free visited states.
  for (it2 = seen.begin(); it2 != seen.end(); it2++)
    delete *it2;
  return count;
}

// Return a set containing all the accepting states in model.
// For each accepting SCC in the product, the states are projected in model.
spot::state_set* project_accepting_states(spot::tgba* prod,
                                    spot::tgba* model)
{
  spot::state_set* res = new spot::state_set;
  spot::scc_map scc_prod (prod);
  scc_prod.build_map();
  typedef std::list<const spot::state*> state_list;
  for (unsigned i = 0; i < scc_prod.scc_count(); ++i)
  {
    if (scc_prod.accepting(i))
    {
      const state_list& slist = scc_prod.states_of(i);
      state_list::const_iterator it;
      for (it = slist.begin(); it != slist.end(); ++it)
      {
        spot::state* proj = prod->project_state(*it, model);
        assert(proj);
        if (res->find(proj) == res->end())
          res->insert(proj);
        else
          delete proj;
      }
    }
  }
  return res;
}

void delete_state_set(spot::state_set* s)
{
  spot::state_set::iterator it;
  for (it = s->begin(); it != s->end(); ++it)
    delete *it;
  delete s;
}

// 'Model checking result cross-comparison test' as described in [TauHel02].
// Given a formula p, the result of model checking must be the same with
// all translation algorithms.
// For each product, compute the set of accepting states in the
// model, this set must be the same for all translation algorithms.
bool check_cross_comparison(tgba_map* m,
                            spot::tgba* model)
{
  tgba_map::iterator it = m->begin();
  // Compute the first state of accepting states as a reference.
  tgba_pair& p = it->second;
  spot::tgba_product* prod = new spot::tgba_product(p.first, model);
  spot::state_set* acc_states_first = project_accepting_states(prod, model);
  delete prod;
  bool good = true;
  for (it = m->begin(); it != m->end() && good; ++it)
  {
    tgba_pair& p = it->second;
    spot::tgba_product* prod = new spot::tgba_product(p.first, model);
    spot::state_set* acc_states = project_accepting_states(prod, model);
    delete prod;
    bool cross_eq = (acc_states->size() == acc_states_first->size());
    // If sizes of sets are different, no need to compare further, they
    // are different, the test failed.
    if (!cross_eq)
      good = false;
    else
    {
      spot::state_set::iterator it;
      // Check if each element in the new set is in the reference set.
      for (it = acc_states->begin(); it != acc_states->end(); ++it)
        if (acc_states_first->find(*it) == acc_states_first->end())
          good = false;
    }
    delete_state_set(acc_states);
  }
  delete_state_set(acc_states_first);
  return true;
}

// 'Model checking result consistency check' as described in [TauHel02].
// Given a formula p, Ap is the resulting Buchi automaton with a given
// translation algorithm.
// Compute the product of the model with Ap and the product
// of the model with A!p.
// For each product, compute the sets Sp and S!p of accepting states in
// the model.  At the end, all states of the model must be either in
// Ap or A!p, i.e Sp U S!p = S (all the states of the model).
bool check_consistency(tgba_map* m,
                       spot::tgba* model)
{
  tgba_map::iterator it;
  for (it = m->begin(); it != m->end(); ++it)
  {
    tgba_pair& p = it->second;
    spot::tgba_product* prod = new spot::tgba_product(p.first, model);
    spot::tgba_product* prod_neg = new spot::tgba_product(p.second, model);
    spot::state_set* acc_states = project_accepting_states(prod, model);
    spot::state_set* acc_states_neg = project_accepting_states(prod_neg, model);
    /*
    // Commented: code to color nodes according to their set.
    std::vector<spot::state_set*> v_set;
    v_set.push_back(acc_states);
    v_set.push_back(acc_states_neg);
    spot::set_dotty_decorator dec(&v_set);
    spot::dotty_reachable(std::cout, model, &dec);
    exit(1);
    */
    spot::state_set* states_union = new spot::state_set;
    // Compute the union Sp U S!p.
    set_union(acc_states->begin(), acc_states->end(),
              acc_states_neg->begin(), acc_states->end(),
              std::inserter(*states_union, states_union->begin()));
              unsigned union_size = states_union->size();
    delete prod;
    delete prod_neg;
    delete_state_set(acc_states);
    delete_state_set(acc_states_neg);
    delete states_union;
    // We check is Sp U S!p = S by simply comparing the sizes.
    if (union_size != tgba_size(model))
      return false;
  }
  return true;
}

void
syntax(char* prog)
{
  std::cerr << "Usage: " << prog << " [OPTIONS] [FORMULA]" << std::endl
	    << std::endl
	    << "Options:" << std::endl
            << "Random Graph Generation Options:" << std::endl
	    << "  -d F   density of the random graph [0.2]" << std::endl
            << "  -n N   number of nodes of the random graph [20]" << std::endl
	    << "  -s N   seed for the random number generator" << std::endl
	    << std::endl
            << "Options for performing emptiness checks:" << std::endl
            << "  -e ALGO   use ALGO for the emptiness check." << std::endl
            << "Where ALGO should be one of:" << std::endl
            << "  Cou99 [default]" << std::endl
            << "  Tau03_opt" << std::endl
            << std::endl
            << "Input options:" << std::endl
            << "  -F filename   use filename as input file. The" << std::endl
            << "                  file should contain one LTL" << std::endl
            << "                  formula per line." << std::endl
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

int
to_int_nonneg(const char* s, const char* arg)
{
  int res = to_int(s);
  if (res < 0)
  {
    std::cerr << "argument of " << arg
              << " (" << res << ") must be nonnegative" << std::endl;
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

float
to_float_nonneg(const char* s, const char* arg)
{
  float res = to_float(s);
  if (res < 0)
  {
    std::cerr << "argument of " << arg
              << " (" << res << ") must be nonnegative" << std::endl;
    exit(1);
  }
  return res;
}

void TimerReset(struct timeval& start)
{
  // Initialize timer with the current time.
  gettimeofday(&start, 0);
}

double TimerGetElapsedTime(struct timeval& start)
{
  double t1, t2;
  struct timeval end;
  TimerReset(end);
  t1 = (double)start.tv_sec + (double)start.tv_usec*0.000001;
  t2 = (double)end.tv_sec + (double)end.tv_usec*0.000001;

  return t2 - t1;
}

int
main(int argc, char** argv)
{
  char empt_default[] = "Cou99";
  char algos_default[] = "fm lacim";
  char* prog = basename(argv[0]);
  char* opt_e = 0;
  int opt_n = 20;
  float opt_d = 0.2;
  char* opt_t = 0;
  char* opt_F = 0;
  bool opt_s = false;
  unsigned seed = 0;

  int argn = 0;
  if (argc == 1)
    syntax(prog);
  while (++argn < argc)
  {
    if (!strcmp(argv[argn], "-e"))
    {
      opt_e = argv[++argn];
    }
    else if (!strcmp(argv[argn], "-n"))
    {
      if (argc < argn + 2)
        syntax(prog);
      opt_n = to_int(argv[++argn]);
    }
    else if (!strcmp(argv[argn], "-d"))
    {
      if (argc < argn + 2)
        syntax(prog);
      opt_d = to_float_nonneg(argv[++argn], "-d");
    }
    else if (!strcmp(argv[argn], "-t"))
    {
      if (argc < argn + 2)
        syntax(prog);
      opt_t = argv[++argn];
    }
    else if (!strcmp(argv[argn], "-F"))
    {
      if (argc < argn + 2)
        syntax(prog);
      opt_F = argv[++argn];
    }
    else if (!strcmp(argv[argn], "-s"))
    {
      if (argc < argn + 2)
        syntax(prog);
      opt_s = true;
      seed = to_int_nonneg(argv[++argn], "-s");
    }
    else
      syntax(prog);
  }
  if (!opt_e)
    opt_e = empt_default;
  if (!opt_s)
    seed = time(0);
  if (!opt_t)
    opt_t = algos_default;
  // List of translation algorithms, will be replaced by
  // ltl_translator_instantiator.
  std::list<std::string> ltl_algos;
  std::list<std::string> chosen_algos;
  ltl_algos.push_back("fm");
  ltl_algos.push_back("lacim");
  ltl_algos.push_back("taa");
  std::istringstream algos(opt_t);
  while (!algos.eof())
  {
    std::string w;
    algos >> std::skipws >> w;
    if (find(ltl_algos.begin(), ltl_algos.end(), w) == ltl_algos.end())
      syntax(prog);
    else
      chosen_algos.push_back(w);
  }
  std::cout << "Seed used: " << seed << std::endl;
  spot::srand(seed);
  spot::bdd_dict* dict = new spot::bdd_dict();
  const char* err;
  spot::emptiness_check_instantiator* inst;
  unsigned inter_success = 0;
  unsigned cons_success = 0;
  unsigned cross_success = 0;
  inst = spot::emptiness_check_instantiator::construct(opt_e, &err);
  spot::ltl::ltl_file formulae(opt_F);
  spot::ltl::formula* f;
  unsigned i = 0;
  struct timeval start;
  TimerReset(start);
  while ((f = formulae.next()))
  {
    std::cout << "Formula " << ++i << ": "
              << spot::ltl::to_string(f) << std::endl;
    spot::ltl::atomic_prop_set* s = spot::ltl::atomic_prop_collect(f, 0);
    spot::tgba* model =
      spot::random_graph(opt_n, opt_d, s, dict, 0, 0.15, 0.5);
    delete s;
    tgba_map* m = build_tgba_map(f, dict, chosen_algos);
    bool intersection_result = check_intersection(m, inst);
    if (intersection_result)
      ++inter_success;
    bool consistency_result = check_consistency(m, model);
    if (consistency_result)
      ++cons_success;
    bool cross_comparison_result = check_cross_comparison(m, model);
    if (cross_comparison_result)
      ++cross_success;
    std::cout << "Check intersection: "
              << std::boolalpha << intersection_result << std::endl;
    std::cout << "Check consistency: "
              << std::boolalpha << consistency_result << std::endl;
    std::cout << "Check cross-comparison: "
              << std::boolalpha << cross_comparison_result << std::endl;
    std::cout << std::endl;
    delete model;
    f->destroy();
    free_tgba_map(m);
  }
  double elapsed = TimerGetElapsedTime(start);
  std::cout << std::endl
            << "Summary: " << std::endl
            << i << " formulae parsed." << std::endl
            << "Intersection check: " << inter_success << "/" << i << std::endl
            << "Consistency check: " << cons_success << "/" << i << std::endl
            << "Cross-comparison check: " << cross_success << "/" << i
            << std::endl
            << "Computation time: " << elapsed << std::endl;
  delete inst;
  delete dict;
}
