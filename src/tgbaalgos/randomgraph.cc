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

#include "randomgraph.hh"
#include "tgba/tgbaexplicit.hh"
#include "misc/random.hh"
#include "ltlast/atomic_prop.hh"
#include "ltlvisit/destroy.hh"
#include <sstream>
#include <list>
#include <set>

namespace spot
{

  namespace
  {
    std::string
    st(int n)
    {
      std::stringstream s;
      s << n;
      return "S" + s.str();
    }

    std::string
    acc(int n)
    {
      std::stringstream s;
      s << n;
      return "a" + s.str();
    }

    void
    random_labels(tgba_explicit* aut,
		  const std::string& src, const std::string& dest,
		  const std::list<int>& props, float t,
		  const std::list<bdd>& accs, float a)
    {
      bdd p = bddtrue;
      for (std::list<int>::const_iterator i = props.begin();
	   i != props.end(); ++i)
	p &= (drand() < t ? bdd_ithvar : bdd_nithvar)(*i);

      bdd ac = bddfalse;
      for (std::list<bdd>::const_iterator i = accs.begin();
	   i != accs.end(); ++i)
	if (drand() < a)
	  ac |= *i;

      tgba_explicit::transition* u = aut->create_transition(src, dest);
      aut->add_conditions(u, p);
      aut->add_acceptance_conditions(u, ac);
    }
  }

  tgba*
  random_graph(int n, float d,
	       const ltl::atomic_prop_set* ap, bdd_dict* dict,
	       int n_acc, float a, float t,
	       ltl::environment* env)
  {
    tgba_explicit* res = new tgba_explicit(dict);

    std::list<int> props;
    for (ltl::atomic_prop_set::const_iterator i = ap->begin();
	 i != ap->end(); ++i)
      props.push_back(dict->register_proposition(*i, res));

    std::list<bdd> accs;
    bdd allneg = bddtrue;
    for (int i = 0; i < n_acc; ++i)
      {
	ltl::formula* f = env->require(acc(i));
	int v = dict->register_acceptance_variable(f, res);
	res->declare_acceptance_condition(f);
	allneg &= bdd_nithvar(v);
	bdd b = bdd_ithvar(v);
	accs.push_back(b);
      }
    for (std::list<bdd>::iterator i = accs.begin(); i != accs.end(); ++i)
      *i &= bdd_exist(allneg, *i);

    typedef std::set<std::string> node_set;
    node_set nodes_to_process;
    node_set unreachable_nodes;

    nodes_to_process.insert(st(0));

    for (int i = 1; i < n; ++i)
      unreachable_nodes.insert(st(i));

    while (!nodes_to_process.empty())
      {
	std::string src = *nodes_to_process.begin();
	nodes_to_process.erase(nodes_to_process.begin());

	if (!unreachable_nodes.empty())
	  {
	    // Pick a random unreachable node.
	    int index = mrand(unreachable_nodes.size());
	    node_set::const_iterator i;
	    for (i = unreachable_nodes.begin(); index; ++i, --index)
	      assert(i != unreachable_nodes.end());

	    // Link it from src.
	    random_labels(res, src, *i, props, t, accs, a);

	    nodes_to_process.insert(*i);
	    unreachable_nodes.erase(i);
	  }
	// Randomly link node to another node (including itself).
	for (int i = 0; i < n; ++i)
	  {
	    if (drand() >= d)
	      continue;

	    std::string dest = st(i);

	    random_labels(res, src, dest, props, t, accs, a);

	    node_set::iterator j = unreachable_nodes.find(dest);
	    if (j != unreachable_nodes.end())
	      {
		nodes_to_process.insert(dest);
		unreachable_nodes.erase(j);
	      }
	  }

	// Avoid dead ends.
	if (res->add_state(src)->empty())
	  random_labels(res, src, src, props, t, accs, a);
      }
    return res;
  }

}
