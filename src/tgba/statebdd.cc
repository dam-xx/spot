#include "statebdd.hh"
#include "bddprint.hh"
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
}
