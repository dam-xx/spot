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

//#define TRACE

#include <iostream>
#ifdef TRACE
#define trace std::cerr
#else
#define trace while (0) std::cerr
#endif

#include <cassert>
#include <utility>
#include <map>
#include <deque>
#include "tgba/tgba.hh"
#include "misc/hash.hh"
#include "emptiness.hh"
#include "emptiness_stats.hh"
#include "gv04.hh"

namespace spot
{
  namespace
  {
    struct stack_entry
    {
      const state* s;		  // State stored in stack entry.
      tgba_succ_iterator* lasttr; // Last transition explored from this state.
      int lowlink;		  // Lowlink value if this entry.
      int pre;			  // DFS predecessor.
      int acc;			  // Accepting state link.
    };

    struct gv04: public emptiness_check, public ec_statistics
    {
      // The automata to check.
      const tgba* a;

      // The unique accepting condition of the automaton \a a,
      // or bddfalse if there is no.
      bdd accepting;

      // Map of visited states.
      typedef Sgi::hash_map<const state*, size_t,
			    state_ptr_hash, state_ptr_equal> hash_type;
      hash_type h;

      // Stack of visited states on the path.
      typedef std::vector<stack_entry> stack_type;
      stack_type stack;

      int top;			// Top of SCC stack.
      int dftop;		// Top of DFS stack.
      bool violation;		// Whether an accepting run was found.

      gv04(const tgba *a)
	: a(a), accepting(a->all_acceptance_conditions())
      {
	assert(a->number_of_acceptance_conditions() <= 1);
      }

      ~gv04()
      {
	for (stack_type::iterator i = stack.begin(); i != stack.end(); ++i)
	  delete i->lasttr;
	hash_type::const_iterator s = h.begin();
	while (s != h.end())
	  {
	    // Advance the iterator before deleting the "key" pointer.
	    const state* ptr = s->first;
	    ++s;
	    delete ptr;
	  }
      }

      virtual emptiness_check_result*
      check()
      {
	top = dftop = -1;
	violation = false;
	push(a->get_init_state(), false);

	while (!violation && dftop >= 0)
	  {
	    trace << "Main iteration (top = " << top
		  << ", dftop = " << dftop
		  << ", s = " << a->format_state(stack[dftop].s)
		  << ")" << std::endl;

	    tgba_succ_iterator* iter = stack[dftop].lasttr;
	    if (!iter)
	      {
		iter = stack[dftop].lasttr = a->succ_iter(stack[dftop].s);
		iter->first();
	      }
	    else
	      {
		iter->next();
	      }

	    if (iter->done())
	      {
		trace << " No more successors" << std::endl;
		pop();
	      }
	    else
	      {
		const state* s_prime = iter->current_state();
		bool acc = iter->current_acceptance_conditions() == accepting;
		inc_transitions();

		trace << " Next successor: s_prime = "
		      << a->format_state(s_prime)
		      << (acc ? " (with accepting link)" : "");

		hash_type::const_iterator i = h.find(s_prime);

		if (i == h.end())
		  {
		    trace << " is a new state." << std::endl;
		    push(s_prime, acc);
		  }
		else
		  {
		    if (i->second < stack.size()
			&& stack[i->second].s->compare(s_prime) == 0)
		      {
			// s_prime has a clone on stack
			trace << " is on stack." << std::endl;
			// This is an addition to GV04 to support TBA.
			violation |= acc;
			lowlinkupdate(dftop, i->second);
		      }
		    else
		      {
			trace << " has been seen, but is no longer on stack."
			      << std::endl;
		      }

		    delete s_prime;
		  }
	      }
	    set_states(h.size());
	  }
	if (violation)
	  return new result(*this);
	return 0;
      }

      void
      push(const state* s, bool accepting)
      {
	trace << "  push(s = " << a->format_state(s)
	      << ", accepting = " << accepting << ")" << std::endl;

	h[s] = ++top;

	stack_entry ss = { s, 0, top, dftop, 0 };

	if (accepting)
	  ss.acc = dftop;	// This differs from GV04 to support TBA.
	else if (dftop >= 0)
	  ss.acc = stack[dftop].acc;
	else
	  ss.acc = -1;

	trace << "    s.lowlink = " << top << std::endl;

	stack.push_back(ss);
	dftop = top;
	inc_depth();
      }

      void
      pop()
      {
	trace << "  pop()" << std::endl;

	int p = stack[dftop].pre;
	if (p >= 0)
	  lowlinkupdate(p, dftop);
	if (stack[dftop].lowlink == dftop)
	  {
	    assert(static_cast<unsigned int>(top + 1) == stack.size());
	    for (int i = top; i >= dftop; --i)
	      {
		delete stack[i].lasttr;
		stack.pop_back();
		dec_depth();
	      }
	    top = dftop - 1;
	  }
	dftop = p;
      }

      void
      lowlinkupdate(int f, int t)
      {
	trace << "  lowlinkupdate(f = " << f << ", t = " << t
	      << ")" << std::endl
	      << "    t.lowlink = " << stack[t].lowlink << std::endl
	      << "    f.lowlink = " << stack[f].lowlink << std::endl;
	int stack_t_lowlink = stack[t].lowlink;
	if (stack_t_lowlink <= stack[f].lowlink)
	  {
	    if (stack_t_lowlink <= stack[f].acc)
	      violation = true;
	    stack[f].lowlink = stack_t_lowlink;
	    trace << "    f.lowlink updated to "
		  << stack[f].lowlink << std::endl;
	  }
      }

      virtual std::ostream&
      print_stats(std::ostream& os) const
      {
	os << h.size() << " unique states visited" << std::endl;
	os << transitions() << " transitions explored" << std::endl;
	os << max_depth() << " items max on stack" << std::endl;
	return os;
      }

      struct result: public emptiness_check_result
      {
	result(gv04& data)
	  : data(data)
	{
	}

	virtual tgba_run*
	accepting_run()
	{
	  tgba_run* res = new tgba_run;

	  // Transitively update the lowlinks, so we can use them in
	  // the BDS bellow.
	  for (int i = 0; i <= data.top; ++i)
	    {
	      int l = data.stack[i].lowlink;
	      if (l < i)
		{
		  int ll = data.stack[i].lowlink = data.stack[l].lowlink;
		  for (int j = i - 1; data.stack[j].lowlink != ll; --j)
		    data.stack[j].lowlink = ll;
		}
	    }
#ifdef TRACE
	  for (int i = 0; i <= data.top; ++i)
	    {
	      trace << "state " << i << " ("
		    << data.a->format_state(data.stack[i].s)
		    << ") has lowlink = " << data.stack[i].lowlink << std::endl;
	    }
#endif

	  // We will use the root of the last SCC as the start of the
	  // cycle.
	  int scc_root = data.stack[data.dftop].lowlink;
	  assert(scc_root >= 0);

	  // Construct the prefix by unwinding the DFS stack before
	  // scc_root.
	  int father = data.stack[scc_root].pre;
	  while (father >= 0)
	    {
	      tgba_run::step st =
		{
		  data.stack[father].s->clone(),
		  data.stack[father].lasttr->current_condition(),
		  data.stack[father].lasttr->current_acceptance_conditions()
		};
	      res->prefix.push_front(st);
	      father = data.stack[father].pre;
	    }

	  // Construct the cycle in two phases.  A first BFS find the
	  // shortest path from scc_root to an accepting transition.
	  // A second BFS then search a path back to scc_root.  If
	  // there is no acceptance conditions we just use the second
	  // BFS to find a cycle around scc_root.
	  const state* bfs_start = data.stack[scc_root].s;
	  const state* bfs_end = bfs_start;
	  if (data.accepting != bddfalse)
	    {
	      trace << "1st BFS" << std::endl;

	      // Records backlinks to parent state during the BFS.
	      // (This also stores the propositions of this link.)
	      std::map<const state*, tgba_run::step,
		       state_ptr_less_than> father;
	      // BFS queue.
	      std::deque<const state*> todo;
	      // Initial state.
	      todo.push_back(bfs_start);

	      while (!todo.empty())
		{
		  const state* src = todo.front();
		  todo.pop_front();
		  tgba_succ_iterator* i = data.a->succ_iter(src);
		  for (i->first(); !i->done(); i->next())
		    {
		      const state* dest = i->current_state();

		      trace << " state " << data.a->format_state(dest);

		      // Do not escape the SCC
		      hash_type::const_iterator j = data.h.find(dest);
		      if (// This state was never visited so far.
			  j == data.h.end()
			  // Or it was discarded
			  || j->second >= data.stack.size()
			  // Or it was discarded (but its stack slot reused)
			  || data.stack[j->second].s->compare(dest)
			  // Or it is still on the stack but not in the SCC
			  || data.stack[j->second].lowlink < scc_root)
			{
			  trace << " ignored" << std::endl;
			  delete dest;
			  continue;
			}
		      trace << " explored" << std::endl;
		      delete dest;
		      dest = j->first;

		      bdd cond = i->current_condition();
		      bdd acc = i->current_acceptance_conditions();
		      tgba_run::step s = { src, cond, acc };

		      if (acc != bddfalse)
			{
			  // Found it!

			  tgba_run::steps p;
			  while (s.s != bfs_start)
			    {
			      p.push_front(s);
			      s = father[s.s];
			    }
			  p.push_front(s);
			  res->cycle.splice(res->cycle.end(), p);
			  // Exit this BFS, and start the next one at dest.
			  todo.clear();
			  bfs_start = dest;
			  break;
			}

		      // Common case: record backlinks and continue BFS
		      // for unvisited states.
		      if (father.find(dest) == father.end())
			{
			  todo.push_back(dest);
			  father[dest] = s;
			}
		    }
		  delete i;
		}
	    }
	  // Second BFS.
	  if (bfs_start != bfs_end || res->cycle.empty())
	    {
	      trace << "2nd BFS" << std::endl;

	      // Records backlinks to parent state during the BFS.
	      // (This also stores the propositions of this link.)
	      std::map<const state*, tgba_run::step,
		state_ptr_less_than> father;
	      // BFS queue.
	      std::deque<const state*> todo;
	      // Initial state.
	      todo.push_back(bfs_start);

	      while (!todo.empty())
		{
		  const state* src = todo.front();
		  todo.pop_front();
		  tgba_succ_iterator* i = data.a->succ_iter(src);
		  for (i->first(); !i->done(); i->next())
		    {
		      const state* dest = i->current_state();

		      trace << " state " << data.a->format_state(dest);

		      // Do not escape the SCC
		      hash_type::const_iterator j = data.h.find(dest);
		      if (// This state was never visited so far.
			  j == data.h.end()
			  // Or it was discarded
			  || j->second >= data.stack.size()
			  // Or it was discarded (but its stack slot reused)
			  || data.stack[j->second].s->compare(dest)
			  // Or it is still on the stack but not in the SCC
			  || data.stack[j->second].lowlink < scc_root)
			{
			  trace << " ignored" << std::endl;
			  delete dest;
			  continue;
			}
		      trace << " explored" << std::endl;
		      delete dest;
		      dest = j->first;

		      bdd cond = i->current_condition();
		      bdd acc = i->current_acceptance_conditions();
		      tgba_run::step s = { src, cond, acc };

		      if (dest == bfs_end)
			{
			  // Found it!
			  tgba_run::steps p;
			  while (s.s != bfs_start)
			    {
			      p.push_front(s);
			      s = father[s.s];
			    }
			  p.push_front(s);
			  res->cycle.splice(res->cycle.end(), p);
			  // Exit this BFS.
			  todo.clear();
			  break;
			}

		      // Common case: record backlinks and continue BFS
		      // for unvisited states.
		      if (father.find(dest) == father.end())
			{
			  todo.push_back(dest);
			  father[dest] = s;
			}
		    }
		  delete i;
		}
	    }

	  // Clone every state in the cycle before returning it.  (We
	  // didn't do that before in the algorithm, because it's
	  // easier to follow if every state manipulated in the BFS is
	  // the instance in the hash table.)
	  for (tgba_run::steps::iterator i = res->cycle.begin();
	       i != res->cycle.end(); ++i)
	    i->s = i->s->clone();

	  assert(res->cycle.begin() != res->cycle.end());

	  return res;
	}

	gv04& data;
      };


    };

  } // anonymous

  emptiness_check*
  explicit_gv04_check(const tgba* a)
  {
    return new gv04(a);
  }
}
