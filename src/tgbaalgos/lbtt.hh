#ifndef SPOT_TGBAALGOS_LBTT_HH
# define SPOT_TGBAALGOS_LBTT_HH

#include "tgba/tgba.hh"
#include <iostream>

namespace spot
{
  /// \brief Print reachable states in LBTT format.
  ///
  /// Note that LBTT expects an automaton with transition
  /// labeled by propositional formulae, and generalized
  /// Büchi accepting conditions on \emph states.  This
  /// is unlike our spot::tgba automata which put
  /// botg generalized accepting conditions and propositional
  /// formulae) on \emph transitions.
  ///
  /// This algorithm will therefore produce an automata
  /// where accepting conditions have been moved from
  /// each transition to previous state.  In the worst
  /// case, doing so will multiply the number of states
  /// and transitions of the automata by <code>2^|Acc|</code>.
  /// where <code>|Acc|</code> is the number of accepting
  /// conditions used by the automata.  You have been warned.
  ///
  /// \param g The automata to print.
  /// \param os Where to print.
  std::ostream& lbtt_reachable(std::ostream& os, const tgba& g);
}

#endif // SPOT_TGBAALGOS_LBTT_HH
