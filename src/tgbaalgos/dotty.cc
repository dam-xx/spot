#include <map>
#include "tgba/tgba.hh"
#include "dotty.hh"
#include "tgba/bddprint.hh"

namespace spot
{
  typedef std::map<int, int> seen_map;

  static bool
  dotty_state(std::ostream& os,
	      const tgba& g, state* st, seen_map& m, int& node)
  {
    bdd s = st->as_bdd();
    seen_map::iterator i = m.find(s.id());

    // Already drawn?
    if (i != m.end())
      {
	node = i->second;
	return false;
      }

    node = m.size() + 1;
    m[s.id()] = node;

    std::cout << "  " << node << " [label=\"";
    bdd_print_set(os, g.get_dict(), s) << "\"]" << std::endl;
    return true;
  }

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
	  dotty_rec(os, g, s, m, node);
	delete s;
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
    delete state;
    return os;
  }


}
