#include <map>
#include "tgba/tgba.hh"
#include "dotty.hh"
#include "tgba/bddprint.hh"

namespace spot
{
  typedef std::map<state*, int, state_ptr_less_than> seen_map;

  /// Output and record a state.
  static bool
  dotty_state(std::ostream& os,
	      const tgba& g, state* st, seen_map& m, int& node)
  {
    seen_map::iterator i = m.find(st);

    // Already drawn?
    if (i != m.end())
      {
	node = i->second;
	return false;
      }

    node = m.size() + 1;
    m[st] = node;

    os << "  " << node << " [label=\""
       << g.format_state(st) << "\"]" << std::endl;
    return true;
  }

  /// Process successors.
  static void
  dotty_rec(std::ostream& os,
	    const tgba& g, state* st, seen_map& m, int father)
  {
    tgba_succ_iterator* si = g.succ_iter(st);
    for (si->first(); !si->done(); si->next())
      {
	int node;
	state* s = si->current_state();
	bool recurse = dotty_state(os, g, s, m, node);
	os << "  " << father << " -> " << node << " [label=\"";
	bdd_print_set(os, g.get_dict(), si->current_condition()) << "\\n";
	bdd_print_set(os, g.get_dict(), si->current_promise()) << "\"]"
							       << std::endl;
	if (recurse)
	  {
	    dotty_rec(os, g, s, m, node);
	    // Do not delete S, it is used as key in M.
	  }
	else
	  {
	    delete s;
	  }
      }
    delete si;
  }

  std::ostream&
  dotty_reachable(std::ostream& os, const tgba& g)
  {
    seen_map m;
    state* state = g.get_init_state();
    os << "digraph G {" << std::endl;
    os << "  size=\"7.26,10.69\"" << std::endl;
    os << "  0 [label=\"\", style=invis]" << std::endl;
    int init;
    dotty_state(os, g, state, m, init);
    os << "  0 -> " << init << std::endl;
    dotty_rec(os, g, state, m, init);
    os << "}" << std::endl;

    // Finally delete all states used as keys in m:
    for (seen_map::iterator i = m.begin(); i != m.end(); ++i)
      delete i->first;

    return os;
  }


}
