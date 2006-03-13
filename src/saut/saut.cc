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

#include "saut.hh"
#include <iostream>

namespace spot
{
  saut::saut(bdd_dict* dict)
    : initial(0), allnegprops(bddtrue), dict(dict)
  {
  }

  void
  saut::set_initial(const node* n)
  {
    initial = n;
    std::cerr << "aut " << this << " uses " << n << " as initial state"
	      << std::endl;
  }

  void
  saut::set_initial(const std::string& name)
  {
    set_initial(declare_node(name));
  }

  saut::node*
  saut::declare_node(const std::string& name)
  {
    std::pair<node_map::iterator, bool> p =
      nodes.insert(std::pair<std::string, node>(name, node()));

    if (p.second)
      {
	p.first->second.name = &p.first->first;
	std::cerr << "aut " << this << " declares "
		  << &p.first->second << " = node `" << name << "'"
		  << std::endl;

	p.first->second.prop_list = bddtrue;
	p.first->second.prop_cond = bddtrue;
	if (!initial)
	  set_initial(&p.first->second);
      }
    return &p.first->second;
  }

  const saut::node*
  saut::known_node(const std::string& name) const
  {
    node_map::const_iterator ir = nodes.find(name);
    if (ir == nodes.end())
      return 0;
    return &ir->second;
  }

  saut::action*
  saut::declare_action(const std::string& name)
  {
    std::pair<action_map::iterator, bool> p =
      actions.insert(std::pair<std::string, action>(name, action()));

    if (p.second)
      {
	p.first->second.name = &p.first->first;
	std::cerr << "aut " << this << " declares "
		  << &p.first->second << " = action `" << name << "'"
		  << std::endl;
      }
    return &p.first->second;
  }

  void
  saut::declare_nodes(const ident_list* idlist)
  {
    for (ident_list::const_iterator i = idlist->begin();
	 i != idlist->end(); ++i)
      declare_node(**i);
  }

  saut::transition*
  saut::declare_transition(const std::string& src,
			   const std::string& act,
			   const std::string& dst)
  {
    node* srcn = declare_node(src);
    action* actn = declare_action(act);
    node* dstn = declare_node(dst);

    srcn->out.push_back(transition(srcn, actn, dstn));
    transition* p = &srcn->out.back();
    dstn->in.push_back(p);
    actn->tia.push_back(p);

    std::cerr << "aut " << this << " declares "
	      << p << " = transition ("
	      << srcn << ", " << actn << ", " << dstn << ")"
	      << std::endl;
    return p;
  }

  void
  saut::declare_propositions(const std::string& str, bdd props)
  {
    node* n = declare_node(str);
    n->prop_list &= props;
    allnegprops -= props;
  }

  void
  saut::declare_propositions(const ident_list* idlist,
			     bdd props)
  {
    for (ident_list::const_iterator i = idlist->begin();
	 i != idlist->end(); ++i)
      declare_propositions(**i, props);
  }

  const saut::action*
  saut::known_action(const action_name& name) const
  {
    action_map::const_iterator i = actions.find(name);
    if (i == actions.end())
      return 0;
    return &i->second;
  }

  const saut::node*
  saut::get_initial() const
  {
    return initial;
  }

  void
  saut::finish()
  {
    for (node_map::iterator i = nodes.begin(); i != nodes.end(); ++i)
      i->second.prop_cond =
	i->second.prop_list & bdd_exist(allnegprops, i->second.prop_list);
  }

  bdd_dict*
  saut::get_dict() const
  {
    return dict;
  }

}
