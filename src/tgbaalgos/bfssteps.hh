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

#ifndef SPOT_TGBAALGOS_BFSSTEPS_HH
# define SPOT_TGBAALGOS_BFSSTEPS_HH

#include <map>
#include <deque>
#include <utility>
#include "emptiness.hh"
#include "tgba/tgba.hh"

namespace spot
{
  /// \brief Make a BFS in a spot::tgba to compute a tgba_run::steps.
  /// \ingroup tgba_misc
  ///
  /// This class should be used to compute the shortest path
  /// between a state of a spot::tgba and the first transition or
  /// state that matches some conditions.
  class bfs_steps
  {
  public:
    bfs_steps(const tgba* a);
    virtual ~bfs_steps();
    const state* search(const state* start, tgba_run::steps& l);
    virtual const state* filter(const state* s) = 0;
    virtual bool match(tgba_run::step& step, const state* dest) = 0;
  protected:
    const tgba* a_;
  };

  template<typename T>
  class bfs_steps_with_path_conditions
  {
  public:
    bfs_steps_with_path_conditions(const tgba* a)
      : a_(a)
    {
    }

    virtual
    ~bfs_steps_with_path_conditions()
    {
    }

    virtual const state* filter(const state* s) = 0;
    virtual bool match(tgba_run::step& step, const state* dest,
		       const T& src_cond, T& dest_cond) = 0;

    const state*
    search(const state* start, tgba_run::steps& l)
    {
      // Records backlinks to parent state during the BFS.
      // (This also stores the propositions of this link.)
      std::map<const state*, tgba_run::step,
	state_ptr_less_than> father;
      // BFS queue.
      std::deque<std::pair<const state*, T> > todo;
      // Initial state.
      todo.push_back(start);

      while (!todo.empty())
	{
	  const state* src = todo.front().first;
	  T cond = todo.front().second;
	  todo.pop_front();
	  tgba_succ_iterator* i = a_->succ_iter(src);
	  for (i->first(); !i->done(); i->next())
	    {
	      const state* dest = filter(i->current_state());

	      if (!dest)
		continue;

	      bdd cond = i->current_condition();
	      bdd acc = i->current_acceptance_conditions();
	      tgba_run::step s = { src, cond, acc };

	      std::pair<const state*, T> next;
	      next.first = src;

	      if (match(s, dest, cond, next.second))
		{
		  // Found it!

		  tgba_run::steps p;
		  for (;;)
		    {
		      tgba_run::step tmp = s;
		      tmp.s = tmp.s->clone();
		      p.push_front(tmp);
		      if (s.s == start)
			break;
		      s = father[s.s];
		    }

		  l.splice(l.end(), p);
		  delete i;
		  return dest;
		}

	      // Common case: record backlinks and continue BFS
	      // for unvisited states.
	      if (father.find(dest) == father.end())
		{
		  todo.push_back(next);
		  father[dest] = s;
		}
	    }
	  delete i;
	}
      return 0;

    }

  protected:
    const tgba* a_;
  };

}

#endif // SPOT_TGBAALGOS_BFSSTEPS_HH
