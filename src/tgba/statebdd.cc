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

  void
  state_bdd::translate(bddPair* rewrite)
  {
    state_ = bdd_replace(state_, rewrite);
  }

  /// Duplicate a state.
  state_bdd*
  state_bdd::clone() const
  {
    return new state_bdd(*this);
  }

}
