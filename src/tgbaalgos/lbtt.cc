#include <map>
#include <set>
#include <string>
#include <sstream>
#include "tgba/tgba.hh"
#include "save.hh"
#include "tgba/bddprint.hh"
#include "ltlvisit/tostring.hh"
#include "tgba/bddprint.hh"

namespace spot
{
  struct bdd_less_than
  {
    bool
    operator()(const bdd& left, const bdd& right) const
    {
      return left.id() < right.id();
    }
  };

  // At some point we'll need to print an accepting set into LBTT's
  // forma.  LBTT expect numbered accepting sets, so first we'll
  // number each accepting condition, and latter when we have to print
  // them we'll just have to look up each of them.
  class accepting_cond_splitter
  {
  public:
    accepting_cond_splitter(bdd all_acc)
    {
      unsigned count = 0;
      while (all_acc != bddfalse)
	{
	  bdd acc = bdd_satone(all_acc);
	  all_acc &= !acc;
	  sm[acc] = count++;
	}
    }

    std::ostream&
    split(std::ostream& os, bdd b)
    {
      while (b != bddfalse)
	{
	  bdd acc = bdd_satone(b);
	  b &= !acc;
	  os << sm[acc] << " ";
	}
      return os;
    }

    unsigned
    count() const
    {
      return sm.size();
    }

  private:
    typedef std::map<bdd, unsigned, bdd_less_than> split_map;
    split_map sm;
  };

  // Convert a BDD formula to the syntax used by LBTT's transition guards.
  // Conjunctions are printed by bdd_format_sat, so we just have
  // to handle the other cases.
  static std::string
  bdd_to_lbtt(bdd b, const tgba_bdd_dict& d)
  {
    if (b == bddfalse)
      return "f";
    else if (b == bddtrue)
      return "t";
    else
      {
	bdd cube = bdd_satone(b);
	b &= !cube;
	if (b != bddfalse)
	  {
	    return "| " + bdd_to_lbtt(b, d) + " " + bdd_to_lbtt(cube, d);
	  }
	else
	  {
	    std::string res = "";
	    for (int count = bdd_nodecount(cube); count > 1; --count)
	      res += "& ";
	    return res + bdd_format_sat(d, cube);
	  }
      }

  }

  // Each state in the produced automata corresponds to
  // a (state, accepting set) pair for the source automata.

  typedef std::pair<state*, bdd> state_acc_pair;

  struct state_acc_pair_less_than
  {
    bool
    operator()(const state_acc_pair& left, const state_acc_pair& right) const
    {
      int cmp = left.first->compare(right.first);
      if (cmp < 0)
	return true;
      if (cmp > 0)
	return false;
      return left.second.id() < right.second.id();
    }
  };

  // Each state of the produced automata is numbered.  Map of state seen.
  typedef std::map<state_acc_pair, unsigned, state_acc_pair_less_than> acp_seen_map;

  // Set of states yet to produce.
  typedef std::set<state_acc_pair, state_acc_pair_less_than> todo_set;

  // Each *source* car correspond to several state in the produced
  // automate.  A minmax_pair specify the range of such associated states.
  typedef std::pair<unsigned, unsigned> minmax_pair;
  typedef std::map<state*, minmax_pair, state_ptr_less_than> seen_map;

  // Take a STATE from the source automaton, and fill TODO with
  // the list of associated states to output.  Return the correponding
  // range in MMP.  Update SEEN, ACP_SEEN, and STATE_NUMBER.
  //
  // INIT must be set to true when registering the initial state.
  // This allows us to create an additional state if required.  (LBTT
  // supports only one initial state, so whenever the initial state
  // of the source automaton has to be split, we need to create
  // a supplementary state, to act as initial state for LBTT.)
  void
  fill_todo(todo_set& todo, seen_map& seen, acp_seen_map& acp_seen,
	    state* state, const tgba& g,
	    minmax_pair& mmp, unsigned& state_number,
	    bool init)
  {
    typedef std::set<bdd, bdd_less_than> bdd_set;

    seen_map::iterator i = seen.find(state);
    if (i != seen.end())
      {
	mmp = i->second;
	delete state;
	return;
      }

    // Browse the successors of STATE to gather accepting
    // conditions of outgoing transitions.
    bdd_set acc_seen;
    tgba_succ_iterator* si = g.succ_iter(state);
    for (si->first(); !si->done(); si->next())
      {
	acc_seen.insert(si->current_accepting_conditions());
      }

    // Order the creation of the supplementary initial state of needed.
    // Use bddtrue as accepting condition because it cannot conflict
    // with other (state, accepting cond) pairs in the maps.
    if (init && acc_seen.size() > 1)
      {
	state_acc_pair p(state, bddtrue);
	todo.insert(p);
	acp_seen[p] = state_number++;
      }

    // Order the creation of normal states.
    mmp.first = state_number;
    for (bdd_set::iterator i = acc_seen.begin(); i != acc_seen.end(); ++i)
      {
	state_acc_pair p(state, *i);
	todo.insert(p);
	acp_seen[p] = state_number++;
      }
    mmp.second = state_number;
    seen[state] = mmp;
  }

  std::ostream&
  lbtt_reachable(std::ostream& os, const tgba& g)
  {
    const tgba_bdd_dict& d = g.get_dict();
    std::ostringstream body;

    seen_map seen;
    acp_seen_map acp_seen;
    todo_set todo;
    unsigned state_number = 0;

    minmax_pair mmp;

    fill_todo(todo, seen, acp_seen,
	      g.get_init_state(), g, mmp, state_number, true);
    accepting_cond_splitter acs(g.all_accepting_conditions());

    while(! todo.empty())
      {
	state_acc_pair sap = *todo.begin();
	todo.erase(todo.begin());
	unsigned number = acp_seen[sap];

	// number == 0 is the initial state.  bddtrue as an accepting
	// conditions indicates a "fake" initial state introduced
	// because the original initial state was split into many
	// states (with different accepting conditions).
	// As this "fake" state has no input transitions, there is
	// no point in computing any accepting conditions.
	body << number << (number ? " 0 " : " 1 ");
	if (sap.second != bddtrue)
	  acs.split(body, sap.second);
	body << "-1" << std::endl;

	tgba_succ_iterator* si = g.succ_iter(sap.first);
	for (si->first(); !si->done(); si->next())
	  {
	    // We have put the accepting conditions on the state,
	    // so draw only outgoing transition with these accepting
	    // conditions.

	    if (sap.second != bddtrue
		&& si->current_accepting_conditions() != sap.second)
	      continue;

	    minmax_pair destrange;
	    fill_todo(todo, seen, acp_seen,
		      si->current_state(), g, destrange, state_number, false);

	    // Link to all instances of the successor.
	    std::string s = bdd_to_lbtt(si->current_condition(), d);
	    for (unsigned i = destrange.first; i < destrange.second; ++i)
	      {
		body << i << " " << s << std::endl;
	      }
	  }
	body << "-1" << std::endl;
	delete si;
      }

    os << state_number << " " << acs.count() << std::endl;
    os << body.str();
    // Finally delete all states used as keys in m:
    for (seen_map::iterator i = seen.begin(); i != seen.end(); ++i)
      delete i->first;
    return os;
  }
}