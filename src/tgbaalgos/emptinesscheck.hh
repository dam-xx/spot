#ifndef SPOT_EMPTINESS_CHECK_HH
# define SPOT_EMPTINESS_CHECK_HH
#include "tgba/tgba.hh"
#include "tgba/statebdd.hh"
#include "tgba/tgbabddfactory.hh"
#include "tgba/succiterconcrete.hh"
#include "tgba/tgbabddconcrete.hh"
#include <map>
#include <stack>
#include <list>
#include <vector>
#include <set>
#include <utility>
#include <ostream>

namespace spot
{

  class connected_component
  {
    // During the Depth path we keep the connected component that we met.
  public:
    connected_component();
    connected_component(int i, bdd a);
    virtual ~connected_component();
    bool isAccepted(tgba* aut);

  public:
    int index;
    /// The bdd condition is the union of all accepting condition of
    /// transitions which connect the states of the connected component.
    bdd condition;
    typedef std::set<const spot::state*,
                     spot::state_ptr_less_than> set_of_state;

    /// for the counter example we need to know all the states of the
    /// component
    set_of_state state_set;
    int transition_acc;
    int nb_transition;
    int nb_state;
    bool not_null;
  };

  class emptiness_check
  {
    typedef std::pair<const spot::state*, tgba_succ_iterator*> pair_state_iter;
    typedef std::pair<pair_state_iter, bdd> triplet;
    typedef std::map<const spot::state*, int, spot::state_ptr_less_than> seen;
    typedef std::list<const state*> state_sequence;
    typedef std::pair<const spot::state*, bdd> state_proposition;
    typedef std::list<state_proposition> cycle_path;

  public:
    /// This function returns true if the automata's language is empty,
    /// and builds a stack of SCC.
    ///
    /// This is based on the following paper.
    /// \verbatim
    /// @InProceedings{couvreur.99.fm,
    ///   author    = {Jean-Michel Couvreur},
    ///   title     = {On-the-fly Verification of Temporal Logic},
    ///   pages     = {253--271},
    ///   editor    = {Jeannette M. Wing and Jim Woodcock and Jim Davies},
    ///   booktitle = {Proceedings of the World Congress on Formal Methods in
    ///                the Development of Computing Systems (FM'99)},
    ///   publisher = {Springer-Verlag},
    ///   series    = {Lecture Notes in Computer Science},
    ///   volume    = {1708},
    ///   year      = {1999},
    ///   address   = {Toulouse, France},
    ///   month     = {September},
    ///   isbn      = {3-540-66587-0}
    /// }
    /// \endverbatim
    bool tgba_emptiness_check(const spot::tgba* aut_check);

    /// Compute a counter example if tgba_emptiness_check() returned false.
    void counter_example(const spot::tgba* aut_counter);

    std::ostream& print_result(std::ostream& os, const spot::tgba* aut,
			       const tgba* restrict = 0) const;

  private:
    std::stack<bdd> arc_accepting;
    std::stack<connected_component> root_component;
    seen seen_state_num;
    state_sequence suffix;
    cycle_path period;

    /// \brief Remove a strongly component from the hash.
    ///
    /// This function remove all accessible state from a given
    /// state. In other words, it removes the strongly connected
    /// component that contains this state.
    void remove_component(const tgba& aut, seen& state_map,
			  const spot::state* start_delete);

    /// Called by counter_example to find a path which traverses all
    /// accepting conditions in the accepted SCC.
    void accepting_path (const spot::tgba* aut_counter,
			 const connected_component& comp_path,
			 const spot::state* start_path, bdd to_accept);

    /// Complete a cycle that caraterise the period of the counter
    /// example.  Append a sequence to the path given by accepting_path.
    void complete_cycle(const spot::tgba* aut_counter,
			const connected_component& comp_path,
			const state* from_state,const state* to_state);
  };
}
#endif // SPOT_EMPTINESS_CHECK_HH
