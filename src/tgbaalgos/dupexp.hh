#ifndef SPOT_TGBAALGOS_DUPEXPL_HH
# define SPOT_TGBAALGOS_DUPEXPL_HH

# include "tgba/tgbaexplicit.hh"

namespace spot
{
  /// Build an explicit automata from all states of \a aut, numbering
  /// states in bread first order as they are processed.
  tgba_explicit* tgba_dupexp_bfs(const tgba* aut);
  /// Build an explicit automata from all states of \a aut, numbering
  /// states in depth first order as they are processed.
  tgba_explicit* tgba_dupexp_dfs(const tgba* aut);
}

#endif // SPOT_TGBAALGOS_DUPEXPL_HH
