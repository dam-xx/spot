#include "statebdd.hh"
#include <cassert>

namespace spot
{
  int
  state_bdd::compare(const state& other) const
  {
    // This method should not be called to compare states from different
    // automata, and all states from the same automaton will use the same
    // state class.
    const state_bdd* o = dynamic_cast<const state_bdd*>(&other);
    assert(o);
    return o->as_bdd().id() - state_.id();
  }

  bdd
  state_bdd::as_bdd() const
  {
    return state_;
  }

}
