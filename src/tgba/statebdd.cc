#include "statebdd.hh"
#include <bdd.h>
#include <cassert>

namespace spot
{

  int
  state_bdd::compare(const state* other) const
  {
    // This method should not be called to compare states from different
    // automata, and all states from the same automaton will use the same
    // state class.
    const state_bdd* o = dynamic_cast<const state_bdd*>(other);
    assert(o);
    return o->as_bdd().id() - state_.id();
  }

  size_t
  state_bdd::hash() const
  {
    return state_.id();
  }

  /// Duplicate a state.
  state_bdd*
  state_bdd::clone() const
  {
    return new state_bdd(*this);
  }

}
