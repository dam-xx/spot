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

#include "reductgba_sim.hh"
#include "tgba/bddprint.hh"

namespace spot
{
  /// Number of spoiler node with a one priority (see icalp2001).
  /// The one priority is represent by a \a acceptance_condition_visited_
  /// which differ of bddfalse.
  /// This spoiler node are looser for the duplicator.
  static int nb_spoiler_loose_;

  static int nb_spoiler;
  static int nb_duplicator;

  //static int nb_node;

  ///////////////////////////////////////////////////////////////////////
  // spoiler_node_delayed

  spoiler_node_delayed::spoiler_node_delayed(const state* d_node,
					     const state* s_node,
					     bdd a,
					     int num)
    : spoiler_node(d_node, s_node, num),
      acceptance_condition_visited_(a)
  {
    nb_spoiler++;
    progress_measure_ = 0;
    if (acceptance_condition_visited_ == bddfalse)
      nb_spoiler_loose_++;
  }

  spoiler_node_delayed::~spoiler_node_delayed()
  {
  }

  bool
  spoiler_node_delayed::set_win()
  {
    // We take the max of the progress measure of the successor node
    // because we are on a spoiler.

    //std::cout << "spoiler_node_delayed::set_win" << std::endl;

    if (lnode_succ->size() == 0)
      progress_measure_ = nb_spoiler_loose_;

    if (progress_measure_ >= nb_spoiler_loose_)
      return false;

    bool change;
    int tmpmax = 0;
    int tmp = 0;
    sn_v::iterator i = lnode_succ->begin();
    if (i != lnode_succ->end())
      {
	tmpmax =
	  dynamic_cast<duplicator_node_delayed*>(*i)->get_progress_measure();
	++i;
      }
    for (; i != lnode_succ->end(); ++i)
      {
	tmp =
	  dynamic_cast<duplicator_node_delayed*>(*i)->get_progress_measure();
	if (tmp > tmpmax)
	  tmpmax = tmp;
      }

    // If the priority of the node is 1
    // acceptance_condition_visited_ != bddfalse
    // then we increment the progress measure of 1.
    if (acceptance_condition_visited_ != bddfalse)
      tmpmax++;

    change = (progress_measure_ < tmpmax);

    progress_measure_ = tmpmax;
    return change;
  }

  std::string
  spoiler_node_delayed::to_string(const tgba* a)
  {
    std::ostringstream os;

    // print the node.
    os << num_
       << " [shape=box, label=\"("
       << a->format_state(sc_->first)
       << ", "
       << a->format_state(sc_->second)
       << ", ";
    //bdd_print_acc(os, a->get_dict(), acceptance_condition_visited_);
    if (acceptance_condition_visited_ == bddfalse)
      {
	os << "false";
      }
    else
      {
	os << "ACC";
      }
    os << ")"
       << " pm = " << progress_measure_ << "\"]"
       << std::endl;

    return os.str();
  }

  bdd
  spoiler_node_delayed::get_acceptance_condition_visited()
  {
    return acceptance_condition_visited_;
  }

  int
  spoiler_node_delayed::get_progress_measure()
  {
    if ((acceptance_condition_visited_ == bddfalse) &&
	(progress_measure_ != (nb_spoiler_loose_ + 1)))
      return 0;
    else
      return progress_measure_;
  }

  ///////////////////////////////////////////////////////////////////////
  // duplicator_node_delayed

  duplicator_node_delayed::duplicator_node_delayed(const state* d_node,
						   const state* s_node,
						   bdd l,
						   bdd a,
						   int num)
    : duplicator_node(d_node, s_node, l, a, num)
  {
    nb_duplicator++;
    progress_measure_ = 0;
  }

  duplicator_node_delayed::~duplicator_node_delayed()
  {
  }

  bool
  duplicator_node_delayed::set_win()
  {
    // We take the min of the progress measure of the successor node
    // because we are on a duplicator.

    //std::cout << "duplicator_node_delayed::set_win" << std::endl;

    //bool debug = true;

    if (progress_measure_ == nb_spoiler_loose_)
      return false;

    bool change;
    int tmpmin = 0;
    int tmp = 0;
    sn_v::iterator i = lnode_succ->begin();
    if (i != lnode_succ->end())
      {
	tmpmin = dynamic_cast<spoiler_node_delayed*>(*i)->get_progress_measure();
	/*
	  debug &= (dynamic_cast<spoiler_node_delayed*>(*i)
	  ->get_acceptance_condition_visited()
	  != bddfalse);
	*/
	++i;
      }
    for (; i != lnode_succ->end(); ++i)
      {
	/*
	  debug &= (dynamic_cast<spoiler_node_delayed*>(*i)
	  ->get_acceptance_condition_visited()
	  != bddfalse);
	*/
	tmp = dynamic_cast<spoiler_node_delayed*>(*i)->get_progress_measure();
	if (tmp < tmpmin)
	  tmpmin = tmp;
      }

    /*
    if (debug)
      std::cout << "All successor p = 1" << std::endl;
    else
      std::cout << "Not All successor p = 1" << std::endl;
    */

    change = (progress_measure_ < tmpmin);
    progress_measure_ = tmpmin;
    return change;
  }

  std::string
  duplicator_node_delayed::to_string(const tgba* a)
  {
    std::ostringstream os;

    // print the node.
    os << num_
       << " [shape=box, label=\"("
       << a->format_state(sc_->first)
       << ", "
       << a->format_state(sc_->second);
    //<< ", ";
    //bdd_print_acc(os, a->get_dict(), acc_);
    os << ")"
       << " pm = " << progress_measure_ << "\"]"
       << std::endl;

    return os.str();
  }

  bool
  duplicator_node_delayed::implies_label(bdd l)
  {
    return ((l | !label_) == bddtrue);
  }

  bool
  duplicator_node_delayed::implies_acc(bdd a)
  {
    return ((a | !acc_) == bddtrue);
  }

  int
  duplicator_node_delayed::get_progress_measure()
  {
    return progress_measure_;
  }

  ///////////////////////////////////////////////////////////////////////
  // parity_game_graph_delayed

  int
  parity_game_graph_delayed::nb_set_acc_cond()
  {
    bdd acc, all;
    acc = all = automata_->all_acceptance_conditions();
    int count = 0;
    while (all != bddfalse)
      {
	sub_set_acc_cond_.push_back(bdd_satone(all));
        all -= bdd_satone(all);
	count++;
      }
    return count;
  }

  void
  parity_game_graph_delayed::build_sub_set_acc_cond()
  {
    // compute the number of acceptance conditions
    bdd acc, all;
    acc = all = automata_->all_acceptance_conditions();
    int count = 0;
    while (all != bddfalse)
      {
	//std::cout << "add acc" << std::endl;
	sub_set_acc_cond_.push_back(bdd_satone(all));
        all -= bdd_satone(all);
	count++;
      }
    // sub_set_acc_cond_ contains all the acceptance condition.
    // but we must have all the sub-set of acceptance condition.
    // In fact we must have 2^count sub-set.

    if (count == 2)
      {
	sub_set_acc_cond_.push_back(acc);
	sub_set_acc_cond_.push_back(bddfalse);
      }

    /*
      bdd_v::iterator i;

      bdd_v::iterator j;
      for (i = sub_set_acc_cond_.begin(); i != sub_set_acc_cond_.end(); ++i)
      for (j = sub_set_acc_cond_.begin(); j != sub_set_acc_cond_.end(); ++j)
      sub_set_acc_cond_.push_back(*i | *j);

      std::cout << std::endl;
      for (i = sub_set_acc_cond_.begin(); i != sub_set_acc_cond_.end();)
      {
      bdd_print_acc(std::cout, automata_->get_dict(), *i);
      std::cout << " // " << std::endl;
      ++i;
      }
      std::cout << std::endl;
    */
  }

  /*
  void
  parity_game_graph_delayed::build_couple()
  {
    //std::cout << "build couple" << std::endl;

    nb_spoiler = 0;
    nb_duplicator = 0;

    tgba_succ_iterator* si = NULL;
    typedef Sgi::pair<bdd, bdd> couple_bdd;
    couple_bdd *p = NULL;
    Sgi::vector<couple_bdd*>* trans = NULL;
    bool exist = false;
    spot::state* s = NULL;

    s_v::iterator i;
    for (i = tgba_state_.begin(); i != tgba_state_.end(); ++i)
      {

	// for each sub-set of the set of acceptance condition.
	bdd_v::iterator i2;
	for (i2 = sub_set_acc_cond_.begin();
	     i2 != sub_set_acc_cond_.end(); ++i2)
	  {

	    // spoiler node are all state couple (i,j)
	    // multiply by 2^(|F|)
	    s_v::iterator i3;
	    for (i3 = tgba_state_.begin();
		 i3 != tgba_state_.end(); ++i3)
	      {
		//nb_spoiler++;
		spoiler_node_delayed* n1
		  = new spoiler_node_delayed(*i,
					     *i3,
					     *i2,
					     nb_node_parity_game++);
		spoiler_vertice_.push_back(n1);
	      }

	    // duplicator node are all state couple where
	    // the first state i are reachable.
	    trans = new Sgi::vector<couple_bdd*>;
	    for (i3 = tgba_state_.begin();
		 i3 != tgba_state_.end(); ++i3)
	      {
		si = automata_->succ_iter(*i3);
		for (si->first(); !si->done(); si->next())
		  {

		    // if there exist a predecessor of i named j
		    s = si->current_state();
		    if (s->compare(*i) == 0)
		      {

			// p is the label of the transition j->i
			p = new couple_bdd(si->current_condition(),
					   si->current_acceptance_conditions());

			// If an other predecessor of i has the same label p
			// to reach i, then we don't compute the
			// duplicator node.
			exist = false;
			Sgi::vector<couple_bdd*>::iterator i4;
			for (i4 = trans->begin();
			     i4 != trans->end(); ++i4)
			  {
			    if ((si->current_condition() == (*i4)->first))
			      // We don't need the acceptance condition
			      //&&
			      //(si->current_acceptance_conditions()
			      //== (*i4)->second))
			      exist = true;
			  }

			if (!exist)
			  {
			    // We build all the state couple with the label p.
			    // multiply by 2^(|F|)
			    trans->push_back(p);
			    Sgi::vector<const state*>::iterator i5;
			    for (i5 = tgba_state_.begin();
				 i5 != tgba_state_.end(); ++i5)
			      {
				//nb_duplicator++;
				int nb = nb_node_parity_game++;
				duplicator_node_delayed* n2
				  = new
				  duplicator_node_delayed(*i,
							  *i5,
							  si->
							  current_condition(),
							  *i2,
							  nb);
				duplicator_vertice_.push_back(n2);
			      }
			  }
			else
			  delete p;
		      }
		    delete s;
		  }
		delete si;
	      }
	    Sgi::vector<couple_bdd*>::iterator i6;
	    for (i6 = trans->begin(); i6 != trans->end(); ++i6)
	      {
		delete *i6;
	      }
	    delete trans;
	  }
      }
    nb_spoiler_loose_++;

    //std::cout << "spoiler    node : " << nb_spoiler << std::endl;
    //std::cout << "duplicator node : " << nb_duplicator << std::endl;
    //std::cout << "nb_spoiler_loose_ : " << nb_spoiler_loose_ << std::endl;
  }

  void
  parity_game_graph_delayed::build_link()
  {
    //std::cout << "build link" << std::endl;
    int nb_ds = 0;
    int nb_sd = 0;
    spot::state* s = NULL;

    // for each couple of (spoiler, duplicator)
    sn_v::iterator i;
    for (i = spoiler_vertice_.begin(); i != spoiler_vertice_.end(); ++i)
      {
	dn_v::iterator i2;
	for (i2 = duplicator_vertice_.begin();
	     i2 != duplicator_vertice_.end(); ++i2)
	  {

	    // We add a link between a duplicator and a spoiler.
	    if ((*i2)->get_spoiler_node()->compare((*i)
						   ->get_spoiler_node()) == 0)
	      {
		tgba_succ_iterator* si
		  = automata_->succ_iter((*i2)->get_duplicator_node());
		for (si->first(); !si->done(); si->next())
		  {
		    s = si->current_state();

		    bdd btmp2 = dynamic_cast<spoiler_node_delayed*>(*i)->
		      get_acceptance_condition_visited();
		    bdd btmp = btmp2 - si->current_acceptance_conditions();

		    //if ((s->compare((*i)->get_duplicator_node()) == 0) &&
		    //dynamic_cast<duplicator_node_delayed*>(*i2)->
		    //  implies_label(si->current_condition()) &&
		    //(btmp == btmp2))

		    if ((s->compare((*i)->get_duplicator_node()) == 0) &&
			dynamic_cast<duplicator_node_delayed*>(*i2)->
			implies_label(si->current_condition()) &&
			(dynamic_cast<spoiler_node_delayed*>(*i)->
			 get_acceptance_condition_visited() != bddfalse))
		      {
			//std::cout << "add duplicator -> spoiler" << std::endl;
			(*i2)->add_succ(*i);
			(*i)->add_pred(*i2);
			nb_ds++;
		      }
		    delete s;
		  }
		delete si;
	      }

	    // We add a link between a spoiler and a duplicator.
	    if ((*i2)->get_duplicator_node()
		->compare((*i)->get_duplicator_node()) == 0)
	      {
		tgba_succ_iterator* si
		  = automata_->succ_iter((*i)->get_spoiler_node());
		for (si->first(); !si->done(); si->next())
		  {
		    s = si->current_state();

		    bdd btmp = si->current_acceptance_conditions() |
		      dynamic_cast<spoiler_node_delayed*>(*i)->
		      get_acceptance_condition_visited();
		    if ((s->compare((*i2)->get_spoiler_node()) == 0) &&
			(*i2)->match(si->current_condition(), btmp))
		      {
			//std::cout << "add spoiler -> duplicator" << std::endl;
			(*i)->add_succ(*i2);
			(*i2)->add_pred(*i);
			nb_sd++;
		      }
		    delete s;
		  }
		delete si;
	      }

	  }
      }
  }
  */

  // We build only node which are reachable
  void
  parity_game_graph_delayed::build_couple()
  {
    // We build only some "basic" spoiler node.

    s_v::iterator i;
    for (i = tgba_state_.begin(); i != tgba_state_.end(); ++i)
      {

	// spoiler node are all state couple (i,j)
	s_v::iterator i2;
	for (i2 = tgba_state_.begin();
	     i2 != tgba_state_.end(); ++i2)
	  {
	    std::cout << "add spoiler node" << std::endl;
	    nb_spoiler++;
	    spoiler_node_delayed* n1
	      = new spoiler_node_delayed(*i, *i2,
					 bddfalse,
					 nb_node_parity_game++);
	    spoiler_vertice_.push_back(n1);
	  }
      }
  }

  void
  parity_game_graph_delayed::build_link()
  {
    // We create when it's possible a duplicator node
    // and recursively his successor.

    //spot::state* s1 = NULL;
    //bool exist_pred = false;

    sn_v::iterator i1;
    for (i1 = spoiler_vertice_.begin(); i1 != spoiler_vertice_.end(); ++i1)
      {
	/*
	exist_pred = false;

	// We check if there is a predecessor only if the duplicator
	// is the initial state.
	s1 = automata_->get_init_state();
	if (s1->compare((*i1)->get_duplicator_node()) == 0)
	  {
	    tgba_succ_iterator* si;
	    s_v::iterator i2;
	    spot::state* s2 = NULL;
	    for (i2 = tgba_state_.begin();
		 i2 != tgba_state_.end(); ++i2)
	      {
		si = automata_->succ_iter(*i2);
		s2 = si->current_state();
		if (s2->compare(s1) == 0)
		  exist_pred = true;
		delete s2;
	      }
	  }
	else
	  exist_pred = true;
	delete s1;

	if (!exist_pred)
	  continue;
	*/

	// We add a link between a spoiler and a (new) duplicator.
	// The acc of the duplicator must contains the
	// acceptance_condition_visited_ of the spoiler.
	build_recurse_successor_spoiler(*i1);

      }

  }

  void
  parity_game_graph_delayed::build_recurse_successor_spoiler(spoiler_node* sn)
  {
    std::cout << "build_recurse_successor_spoiler" << std::endl;

    tgba_succ_iterator* si = automata_->succ_iter(sn->get_spoiler_node());

    int i = 0;
    for (si->first(); !si->done(); si->next())
      {
	std::cout << "transition " << i++ << std::endl;

	bdd btmp = si->current_acceptance_conditions() |
	  dynamic_cast<spoiler_node_delayed*>(sn)->
	  get_acceptance_condition_visited();

	s_v::iterator i1;
	state* s;
	for (i1 = tgba_state_.begin();
	     i1 != tgba_state_.end(); ++i1)
	  {
	    s  = si->current_state();
	    if (s->compare(*i1) == 0)
	      {
		duplicator_node_delayed* dn
		  = new duplicator_node_delayed(*i1,
						sn->get_duplicator_node(),
						si->current_condition(),
						btmp,
						nb_node_parity_game++);
		duplicator_vertice_.push_back(dn);

		// dn is already a successor of sn.
		if (!(sn->add_succ(dn)))
		  continue;
		(dn)->add_pred(sn);

		/* TEST
		   bdd btmp2 =
		   dynamic_cast<spoiler_node_delayed*>(sn)->
		   get_acceptance_condition_visited();
		*/

		build_recurse_successor_duplicator(dn, sn);
	      }
	    delete s;
	  }
      }

    delete si;

  }

  void
  parity_game_graph_delayed::
  build_recurse_successor_duplicator(duplicator_node* dn,
				     spoiler_node* sn)
  {
    std::cout << "build_recurse_successor_duplicator" << std::endl;

    tgba_succ_iterator* si = automata_->succ_iter(dn->get_duplicator_node());

    int i = 0;
    for (si->first(); !si->done(); si->next())
      {
	std::cout << "transition " << i++ << std::endl;

	bdd btmp =
	  dynamic_cast<spoiler_node_delayed*>(sn)->
	    get_acceptance_condition_visited();
	bdd btmp2 = btmp - si->current_acceptance_conditions();

	s_v::iterator i1;
	state* s;
	for (i1 = tgba_state_.begin();
	     i1 != tgba_state_.end(); ++i1)
	  {
	    s  = si->current_state();
	    if (s->compare(*i1) == 0)
	      {
		spoiler_node_delayed* sn_n
		  = new spoiler_node_delayed(sn->get_spoiler_node(),
					     *i1,
					     btmp2,
					     nb_node_parity_game++);
		spoiler_vertice_.push_back(sn_n);

		// dn is already a successor of sn.
		if (!(dn->add_succ(sn_n)))
		  continue;
		(sn_n)->add_pred(dn);

		build_recurse_successor_spoiler(sn_n);

	      }
	    delete s;
	  }
      }

    delete si;

  }


  void
  parity_game_graph_delayed::add_dup_node(state*,
					  state*,
					  bdd,
					  bdd)
  {
  }

  void
  parity_game_graph_delayed::prune()
  {

    bool change = true;

    while (change)
      {
	std::cout << "prune::change = true" << std::endl;
	change = false;
	for (Sgi::vector<duplicator_node*>::iterator i
	       = duplicator_vertice_.begin();
	     i != duplicator_vertice_.end();)
	  {
	    if ((*i)->get_nb_succ() == 0)
	      {
		(*i)->del_pred();
		delete *i;
		i = duplicator_vertice_.erase(i);
		change = true;
	      }
	    else
	      ++i;
	  }
	for (Sgi::vector<spoiler_node*>::iterator i
	       = spoiler_vertice_.begin();
	     i != spoiler_vertice_.end();)
	  {
	    if ((*i)->get_nb_succ() == 0)
	      {
		(*i)->del_pred();
		delete *i;
		i = spoiler_vertice_.erase(i);
		change = true;
	      }
	    else
	      ++i;
	  }
      }
    std::cout << "prune::change = false" << std::endl;
  }

  void
  parity_game_graph_delayed::lift()
  {
    // Jurdzinski's algorithm
    //int iter = 0;
    bool change = true;

    while (change)
      {
	std::cout << "lift::change = true" << std::endl;
	change = false;
	for (Sgi::vector<duplicator_node*>::iterator i
	       = duplicator_vertice_.begin();
	     i != duplicator_vertice_.end(); ++i)
	  {
	    change |= (*i)->set_win();
	  }
	for (Sgi::vector<spoiler_node*>::iterator i
	       = spoiler_vertice_.begin();
	     i != spoiler_vertice_.end(); ++i)
	  {
	    change |= (*i)->set_win();
	  }
      }
    std::cout << "lift::change = false" << std::endl;
  }

  simulation_relation*
  parity_game_graph_delayed::get_relation()
  {
    simulation_relation* rel = new simulation_relation();
    state_couple* p = NULL;
    seen_map::iterator j;

    for (Sgi::vector<spoiler_node*>::iterator i
	   = spoiler_vertice_.begin();
	 i != spoiler_vertice_.end(); ++i)
      {
	if (dynamic_cast<spoiler_node_delayed*>(*i)->get_progress_measure()
	    < nb_spoiler_loose_)
	  {
	    p = new state_couple((*i)->get_spoiler_node(),
				 (*i)->get_duplicator_node());
	    rel->push_back(p);

	    // We remove the state in rel from seen
	    // because the destructor of
	    // tgba_reachable_iterator_breadth_first
	    // delete all the state.

	    if ((j = seen.find(p->first)) != seen.end())
	      seen.erase(j);
	    if ((j = seen.find(p->second)) != seen.end())
	      seen.erase(j);
	  }

      }

    return rel;
  }

  parity_game_graph_delayed::~parity_game_graph_delayed()
  {
  }

  parity_game_graph_delayed::parity_game_graph_delayed(const tgba* a)
    : parity_game_graph(a)
  {
    nb_spoiler_loose_ = 0;
    /*
      if (this->nb_set_acc_cond() > 2)
      return;
      this->build_sub_set_acc_cond();
    */
    std::cout << "build couple" << std::endl;
    this->build_couple();
    std::cout << "build link" << std::endl;
    this->build_link();
    std::cout << "prune" << std::endl;
    this->prune();
    std::cout << "lift" << std::endl;
    this->lift();
    std::cout << "END" << std::endl;
    //this->print(std::cout);
  }

  ///////////////////////////////////////////
  simulation_relation*
  get_delayed_relation_simulation(const tgba* f, int opt)
  {
    /// FIXME : this method is incorrect !!
    /// Don't use it !!
    parity_game_graph_delayed* G = new parity_game_graph_delayed(f);
    simulation_relation* rel = G->get_relation();
    if (opt == 1)
      G->print(std::cout);
    delete G;
    return rel;
  }

}
