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
#include "tgba/tgba.hh"
#include "misc/hash.hh"
#include "emptiness.hh"
#include "gv04.hh"

namespace spot
{
  namespace
  {
    struct stack_entry
    {
      const state* s;		  // State stored in stack entry.
      tgba_succ_iterator* nexttr; // Next transition to explore.
				  // (The paper uses lasttr for the
				  // last transition, but nexttr is
                                  // easier for our iterators.)
      int lowlink;		  // Lowlink value if this entry.
      int pre;			  // DFS predecessor.
      int acc;			  // Accepting state link.
    };

    struct gv04: public emptiness_check
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
      size_t max_stack_size;

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
	  delete i->nexttr;
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
	max_stack_size = 0;
	top = dftop = -1;
	violation = false;
	push(a->get_init_state(), false);

	while (!violation && dftop >= 0)
	  {
	    tgba_succ_iterator* iter = stack[dftop].nexttr;

	    trace << "Main iteration (top = " << top
		  << ", dftop = " << dftop
		  << ", s = " << a->format_state(stack[dftop].s)
		  << ")" << std::endl;

	    if (iter->done())
	      {
		trace << " No more successors" << std::endl;
		pop();
	      }
	    else
	      {
		const state* s_prime = iter->current_state();
		bool acc = iter->current_acceptance_conditions() == accepting;
		iter->next();

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
	  }
	if (violation)
	  return new emptiness_check_result;
	return 0;
      }

      void
      push(const state* s, bool accepting)
      {
	trace << "  push(s = " << a->format_state(s)
	      << ", accepting = " << accepting << ")" << std::endl;

	h[s] = ++top;

	tgba_succ_iterator* iter = a->succ_iter(s);
	iter->first();
	stack_entry ss = { s, iter, top, dftop, 0 };

	if (accepting)
	  ss.acc = dftop;	// This differs from GV04 to support TBA.
	else if (dftop >= 0)
	  ss.acc = stack[dftop].acc;
	else
	  ss.acc = -1;

	trace << "    s.lowlink = " << top << std::endl;

	stack.push_back(ss);
	dftop = top;

	max_stack_size = std::max(max_stack_size, stack.size());
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
		delete stack[i].nexttr;
		stack.pop_back();
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
	os << max_stack_size << " items max on stack" << std::endl;
	return os;
      }

    };

  } // anonymous

  emptiness_check*
  explicit_gv04_check(const tgba* a)
  {
    return new gv04(a);
  }
}
