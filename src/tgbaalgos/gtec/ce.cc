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

#include "ce.hh"
#include "tgbaalgos/bfssteps.hh"

namespace spot
{
  couvreur99_check_result::couvreur99_check_result
  (const couvreur99_check_status* ecs,
   const explicit_connected_component_factory* eccf)
    : ecs_(ecs), eccf_(eccf)
  {
  }

  tgba_run*
  couvreur99_check_result::accepting_run()
  {
    run_ = new tgba_run;

    assert(!ecs_->root.empty());

    scc_stack::stack_type root = ecs_->root.s;
    int comp_size = root.size();
    // Transform the stack of connected component into an array.
    explicit_connected_component** scc =
      new explicit_connected_component*[comp_size];
    for (int j = comp_size - 1; 0 <= j; --j)
      {
	scc[j] = eccf_->build();
	scc[j]->index = root.top().index;
	scc[j]->condition = root.top().condition;
	root.pop();
      }
    assert(root.empty());

    // Build the set of states for all SCCs.
    numbered_state_heap_const_iterator* i = ecs_->h->iterator();
    for (i->first(); !i->done(); i->next())
      {
	int index = i->get_index();
	// Skip states from dead SCCs.
	if (index < 0)
	  continue;
	assert(index != 0);

	// Find the SCC this state belongs to.
	int j;
	for (j = 1; j < comp_size; ++j)
	  if (index < scc[j]->index)
	    break;
	scc[j - 1]->insert(i->get_state());
      }
    delete i;

    numbered_state_heap::state_index_p spi =
      ecs_->h->index(ecs_->aut->get_init_state());
    assert(spi.first);

    // We build a path trough each SCC in the stack.  For the
    // first SCC, the starting state is the initial state of the
    // automaton.  The destination state is the closest state
    // from the next SCC.  This destination state becomes the
    // starting state when building a path through the next SCC.
    const state* start = spi.first;
    for (int k = 0; k < comp_size - 1; ++k)
      {

	struct scc_bfs: bfs_steps
	{
	  explicit_connected_component** scc;
	  int k;
	  bool in_next;
	  scc_bfs(const tgba* a, explicit_connected_component** scc, int k)
	    : bfs_steps(a), scc(scc), k(k)
	  {
	  }

	  virtual const state*
	  filter(const state* s)
	  {
	    const state* h_s = scc[k]->has_state(s);
	    if (!h_s)
	      {
		h_s = scc[k+1]->has_state(s);
		in_next = true;
		if (!h_s)
		  delete s;
	      }
	    else
	      {
		in_next = false;
	      }
	    return h_s;
	  }

	  virtual bool
	  match(tgba_run::step&, const state*)
	  {
	    return in_next;
	  }


	} b(ecs_->aut, scc, k);

	start = b.search(start, run_->prefix);
      }

    accepting_cycle(scc[comp_size - 1], start,
		    scc[comp_size - 1]->condition);

    for (int j = comp_size - 1; 0 <= j; --j)
      delete scc[j];
    delete[] scc;

    return run_;
  }

  namespace
  {
    struct triplet
    {
      const state* s;		// Current state.
      tgba_succ_iterator* iter;	// Iterator to successor of the current state.
      bdd acc;			// All acceptance conditions traversed by
				// the path so far.

      triplet (const state* s, tgba_succ_iterator* iter, bdd acc)
	: s(s), iter(iter), acc(acc)
      {
      }
    };

  }

  void
  couvreur99_check_result::accepting_cycle(const explicit_connected_component*
					   scc,
					   const state* start, bdd
					   acc_to_traverse)
  {
    // Compute an accepting cycle using successive BFS that are
    // restarted from the point reached after we have discovered a
    // transition with a new acceptance conditions.
    //
    // This idea is taken from Product<T>::findWitness in LBTT 1.1.2.
    const state* substart = start;
    do
      {
	struct scc_bfs: bfs_steps
	{
	  const explicit_connected_component* scc;
	  bool in_next;
	  bdd& acc_to_traverse;
	  const state* start;
	  scc_bfs(const tgba* a, const explicit_connected_component* scc,
		  bdd& acc_to_traverse, const state* start)
	    : bfs_steps(a), scc(scc), acc_to_traverse(acc_to_traverse),
	      start(start)
	  {
	  }

	  virtual const state*
	  filter(const state* s)
	  {
	    const state* h_s = scc->has_state(s);
	    if (!h_s)
	      delete s;
	    return h_s;
	  }

	  virtual bool
	  match(tgba_run::step& st, const state* s)
	  {
	    bdd less_acc = acc_to_traverse - st.acc;
	    if (less_acc != acc_to_traverse
		|| (acc_to_traverse == bddfalse
		    && s == start))
	      {
		acc_to_traverse = less_acc;
		return true;
	      }
	    return false;
	  }

	} b(ecs_->aut, scc, acc_to_traverse, start);

	substart = b.search(substart, run_->cycle);
      }
    while (acc_to_traverse != bddfalse || substart != start);
  }

  void
  couvreur99_check_result::print_stats(std::ostream& os) const
  {
    ecs_->print_stats(os);
    // FIXME: This is bogusly assuming run_ exists.  (Even if we
    // created it, the user might have delete it.)
    os << run_->prefix.size() << " states in run_->prefix" << std::endl;
    os << run_->cycle.size() << " states in run_->cycle" << std::endl;
  }

}
