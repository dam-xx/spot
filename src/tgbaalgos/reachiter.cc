#include <cassert>
#include "reachiter.hh"

namespace spot
{
  // tgba_reachable_iterator
  //////////////////////////////////////////////////////////////////////

  tgba_reachable_iterator::tgba_reachable_iterator(const tgba* a)
    : automata_(a)
  {
  }

  tgba_reachable_iterator::~tgba_reachable_iterator()
  {
    for (seen_map::const_iterator s = seen.begin(); s != seen.end(); ++s)
      delete s->first;
  }

  void
  tgba_reachable_iterator::run()
  {
    int n = 0;
    start();
    state* i = automata_->get_init_state();
    add_state(i);
    seen[i] = ++n;
    const state* t;
    while ((t = next_state()))
      {
	assert(seen.find(t) != seen.end());
	int tn = seen[t];
	tgba_succ_iterator* si = automata_->succ_iter(t);
	process_state(t, tn, si);
	for (si->first(); ! si->done(); si->next())
	  {
	    const state* current = si->current_state();
	    seen_map::const_iterator s = seen.find(current);
	    if (s == seen.end())
	      {
		seen[current] = ++n;
		add_state(current);
		process_link(tn, n, si);
	      }
	    else
	      {
		process_link(tn, seen[current], si);
		delete current;
	      }
	  }
	delete si;
      }
    end();
  }

  void
  tgba_reachable_iterator::start()
  {
  }

  void
  tgba_reachable_iterator::end()
  {
  }

  void
  tgba_reachable_iterator::process_state(const state*, int,
					 tgba_succ_iterator*)
  {
  }

  void
  tgba_reachable_iterator::process_link(int, int, const tgba_succ_iterator*)
  {
  }

  // tgba_reachable_iterator_depth_first
  //////////////////////////////////////////////////////////////////////

  tgba_reachable_iterator_depth_first::
    tgba_reachable_iterator_depth_first(const tgba* a)
      : tgba_reachable_iterator(a)
  {
  }

  void
  tgba_reachable_iterator_depth_first::add_state(const state* s)
  {
    todo.push(s);
  }

  const state*
  tgba_reachable_iterator_depth_first::next_state()
  {
    if (todo.empty())
      return 0;
    const state* s = todo.top();
    todo.pop();
    return s;
  }

  // tgba_reachable_iterator_breadth_first
  //////////////////////////////////////////////////////////////////////

  tgba_reachable_iterator_breadth_first::
    tgba_reachable_iterator_breadth_first(const tgba* a)
      : tgba_reachable_iterator(a)
  {
  }

  void
  tgba_reachable_iterator_breadth_first::add_state(const state* s)
  {
    todo.push_back(s);
  }

  const state*
  tgba_reachable_iterator_breadth_first::next_state()
  {
    if (todo.empty())
      return 0;
    const state* s = todo.front();
    todo.pop_front();
    return s;
  }

}
