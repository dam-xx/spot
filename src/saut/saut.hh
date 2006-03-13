// Copyright (C) 2006 Laboratoire d'Informatique de
// Paris 6 (LIP6), département Systèmes Répartis Coopératifs (SRC),
// Université Pierre et Marie Curie.
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

#ifndef SPOT_SAUT_HH
# define SPOT_SAUT_HH

#include <list>
#include <string>
#include <map>
#include <set>
#include "tgba/bdddict.hh"

namespace spot
{

  class saut
  {
  public:
    typedef std::string node_name;
    typedef std::string action_name;

    struct node;
    struct action;

    struct transition
    {
      node* src;
      action* act;
      node* dst;
      transition(node* src, action* act, node* dst)
	: src(src), act(act), dst(dst) {}
    };

    typedef std::list<transition> transitions_list;
    typedef std::list<transition*> transitionsp_list;

    struct node
    {
      const node_name* name;
      transitions_list out;
      transitionsp_list in;
      bdd props;
    };

    struct action
    {
      const action_name* name;
      transitionsp_list tia;
    };

    typedef std::map<node_name, node> node_map;
    typedef std::map<action_name, action> action_map;
    typedef std::list<const std::string*> ident_list;

  private:
    node_map nodes;
    action_map actions;
    const node* initial;
    bdd allnegprops;
    bdd_dict* dict;
  public:
    saut(bdd_dict* dict);
    bdd_dict* get_dict() const;
    void set_initial(const node* n);
    void set_initial(const std::string& name);

    node* declare_node(const std::string& name);
    void declare_nodes(const ident_list* idlist);
    action* declare_action(const std::string& name);
    transition* declare_transition(const std::string& src,
				   const std::string& act,
				   const std::string& dst);
    const action* known_action(const action_name& name) const;
    const node* known_node(const std::string& name) const;
    void declare_propositions(const std::string& name, bdd props);
    void declare_propositions(const ident_list* idlist, bdd props);

    void finish();

    const node* get_initial() const;
  };
}

#endif // SPOT_SAUT_HH
