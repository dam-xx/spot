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


#ifndef SPOT_REDUC_TGBA_SIM_HH
#define SPOT_REDUC_TGBA_SIM_HH

#include "tgba/tgbareduc.hh"
#include "tgbaalgos/reachiter.hh"
#include <vector>
#include <list>
#include <sstream>

namespace spot
{

  /// Options for reduce.
  enum reduce_tgba_options
    {
      /// No reduction.
      Reduce_None = 0,
      /// Reduction using direct simulation relation.
      Reduce_Dir_Sim = 1,
      /// Reduction using delayed simulation relation.
      Reduce_Del_Sim = 2,
      /// Reduction using SCC.
      Reduce_Scc = 4,
      /// All reductions.
      Reduce_All = -1U
    };

  /// \brief Remove some node of the automata using a simulation
  /// relation.
  ///
  /// \param a the automata to reduce.
  /// \param opt a conjonction of spot::reduce_tgba_options specifying
  //             which optimizations to apply.
  /// \return the reduced automata.
  tgba* reduc_tgba_sim(const tgba* a, int opt = Reduce_All);

  /// \brief Compute a direct simulation relation on state of tgba \a f.
  simulation_relation* get_direct_relation_simulation(const tgba* a,
						      int opt = -1);

  /// Compute a delayed simulation relation on state of tgba \a f.
  /// FIXME : this method is incorrect !!
  /// Don't use it !!
  simulation_relation* get_delayed_relation_simulation(const tgba* a,
						      int opt = -1);

  /// To free a simulation relation.
  void free_relation_simulation(simulation_relation* rel);

  /// Test if the initial state of a2 fair simulate this of a1.
  /// Not implemented.
  bool is_include(const tgba* a1, const tgba* a2);

  ///////////////////////////////////////////////////////////////////////
  // simulation.

  class spoiler_node;
  class duplicator_node;

  typedef Sgi::vector<spoiler_node*> sn_v;
  typedef Sgi::vector<duplicator_node*> dn_v;
  typedef Sgi::vector<const state*> s_v;

  /// \brief Parity game graph which compute a simulation relation.
  class parity_game_graph : public tgba_reachable_iterator_breadth_first
  {
  public:
    parity_game_graph(const tgba* a);
    virtual ~parity_game_graph();

    virtual simulation_relation* get_relation() = 0;

    void print(std::ostream& os);

  protected:
    sn_v spoiler_vertice_;
    dn_v duplicator_vertice_;
    s_v tgba_state_;
    int nb_node_parity_game;

    void start();
    void end();
    void process_state(const state* s, int n, tgba_succ_iterator* si);
    void process_link(int in, int out, const tgba_succ_iterator* si);

    /// \brief Compute each node of the graph.
    virtual void build_graph() = 0;

    /// \brief Compute the link of the graph.
    /// Successor of spoiler node (resp. duplicator node)
    /// are duplicator node (resp. spoiler node).
    //virtual void build_link() = 0;

    /// \brief Remove edge from spoiler to duplicator that make
    /// duplicator loose.
    /// Spoiler node whose still have some link, reveal
    /// a direct simulation relation.
    virtual void lift() = 0;
  };

  ///////////////////////////////////////////////////////////////////////
  // Direct simulation.

  /// Spoiler node of parity game graph.
  class spoiler_node
  {
  public:
    spoiler_node(const state* d_node,
		 const state* s_node,
		 int num);
    virtual ~spoiler_node();

    /// \brief Add a successor.
    /// Return true if \a n wasn't yet in the list of successor,
    /// false eitherwise.
    bool add_succ(spoiler_node* n);
    void del_succ(spoiler_node* n);
    virtual void add_pred(spoiler_node* n);
    virtual void del_pred();
    int get_nb_succ();
    bool prune();
    virtual bool set_win();
    virtual std::string to_string(const tgba* a);
    virtual std::string succ_to_string();
    virtual bool compare(spoiler_node* n);

    const state* get_spoiler_node();
    const state* get_duplicator_node();
    state_couple* get_pair();

    bool not_win;
    int num_; // for the dot display.

  protected:
    sn_v* lnode_succ;
    sn_v* lnode_pred;
    //Sgi::vector<spoiler_node*>* lnode_succ;
    state_couple* sc_;
  };

  /// Duplicator node of parity game graph.
  class duplicator_node : public spoiler_node
  {
  public:
    duplicator_node(const state* d_node,
		    const state* s_node,
		    bdd l,
		    bdd a,
		    int num);
    virtual ~duplicator_node();

    virtual bool set_win();
    virtual std::string to_string(const tgba* a);
    virtual bool compare(spoiler_node* n);

    bool match(bdd l, bdd a);
    bool implies(bdd l, bdd a);

    bdd get_label() const;
    bdd get_acc() const;

  protected:
    bdd label_;
    bdd acc_;
  };

  /// Parity game graph which compute the direct simulation relation.
  class parity_game_graph_direct : public parity_game_graph
  {
  public:
    parity_game_graph_direct(const tgba* a);
    ~parity_game_graph_direct();

    virtual simulation_relation* get_relation();

  protected:
    virtual void build_graph();
    virtual void lift();
    void build_link();

    /*
      private:
      void build_recurse_successor_spoiler(spoiler_node* sn,
      std::ostringstream& os);
      void build_recurse_successor_duplicator(duplicator_node* dn,
      spoiler_node* sn,
      std::ostringstream& os);
      duplicator_node* add_duplicator_node(const spot::state* sn,
      const spot::state* dn,
      bdd acc,
      bdd label,
      int nb);
      spoiler_node* add_spoiler_node(const spot::state* sn,
      const spot::state* dn,
      int nb);
    */

  };


  ///////////////////////////////////////////////////////////////////////
  // Delayed simulation.

  /// Spoiler node of parity game graph for delayed simulation.
  class spoiler_node_delayed : public spoiler_node
  {
  public:
    spoiler_node_delayed(const state* d_node,
			 const state* s_node,
			 bdd a,
			 int num,
			 bool l2a = true);
    ~spoiler_node_delayed();

    /// Return true if the progress_measure has changed.
    bool set_win();
    bdd get_acceptance_condition_visited() const;
    virtual bool compare(spoiler_node* n);
    virtual std::string to_string(const tgba* a);
    int get_progress_measure() const;

    bool get_lead_2_acc_all();
    /*
    void set_lead_2_acc_all();
    */
  protected:
    /// a Bdd for retain all the acceptance condition
    /// that a node has visited.
    bdd acceptance_condition_visited_;
    int progress_measure_;
    bool lead_2_acc_all_;

  };

  /// Duplicator node of parity game graph for delayed simulation.
  class duplicator_node_delayed : public duplicator_node
  {
  public:
    duplicator_node_delayed(const state* d_node,
			    const state* s_node,
			    bdd l,
			    bdd a,
			    int num);
    ~duplicator_node_delayed();

    /// Return true if the progress_measure has changed.
    bool set_win();
    virtual std::string to_string(const tgba* a);
    bool implies_label(bdd l);
    bool implies_acc(bdd a);
    int get_progress_measure();
    bool get_lead_2_acc_all();
    void set_lead_2_acc_all();

  protected:
    int progress_measure_;
    bool lead_2_acc_all_;
  };

  /// Parity game graph which compute the delayed simulation relation
  /// as explain in
  /// @inproceedings{ icalp2001,
  /// AUTHOR = {Etessami, Thomas Wilke, Rebecca A. Schuller},
  /// TITLE = {Fair Simulation Relations, Parity Games, and State Space
  ///          Reduction for Buchi Automata},
  /// BOOKTITLE = {Automata, Languages and Programming,
  /// 28th international collquium},
  /// PAGES = {694--707},
  /// YEAR = 2001,
  /// EDITOR = {Orejas, Fernando and Spirakis, Paul G. and van Leeuwen, Jan},
  /// VOLUME = 2076,
  /// SERIES = {Lecture Notes in Computer Science},
  /// ADDRESS = {Crete, Greece},
  /// MONTH = JUL,
  /// PUBLISHER = {Springer},
  /// url = {citeseer.ist.psu.edu/472661.html}
  /// }

  class parity_game_graph_delayed : public parity_game_graph
  {
  public:
    parity_game_graph_delayed(const tgba* a);
    ~parity_game_graph_delayed();

    virtual simulation_relation* get_relation();

  private:

    /// Vector which contain all the sub-set of the set
    /// of acceptance condition.
    typedef Sgi::vector<bdd> bdd_v;
    bdd_v sub_set_acc_cond_;

    /// Return the number of acceptance condition.
    int nb_set_acc_cond();

    /// Compute sub_set_acc_cond_;
    void build_sub_set_acc_cond();

    ///
    duplicator_node_delayed* add_duplicator_node_delayed(const spot::state* sn,
							 const spot::state* dn,
							 bdd acc,
							 bdd label,
							 int nb);

    ///
    spoiler_node_delayed* add_spoiler_node_delayed(const spot::state* sn,
						   const spot::state* dn,
						   bdd acc,
						   int nb);

    /// \brief Compute the couple as for direct simulation,
    virtual void build_graph();
    //virtual void build_link();

    void build_recurse_successor_spoiler(spoiler_node* sn,
					 std::ostringstream& os);
    void build_recurse_successor_duplicator(duplicator_node* dn,
					    spoiler_node* sn,
					    std::ostringstream& os);

    /// \brief The Jurdzinski's lifting algorithm.
    virtual void lift();

    /// \brief Remove all node so as to there is no dead ends (terminal node).
    //virtual void prune();
  };

}

#endif
