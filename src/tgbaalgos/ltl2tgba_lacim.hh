#ifndef SPOT_TGBA_LTL2TGBA_HH
# define SPOT_TGBA_LTL2TGBA_HH

#include "ltlast/formula.hh"
#include "tgba/tgbabddconcrete.hh"

namespace spot
{
  /// Build a spot::tgba_bdd_concrete from an LTL formula.
  ///
  /// This is based on the following paper.
  /// \verbatim
  /// @InProceedings{   couvreur.00.lacim,
  ///   author        = {Jean-Michel Couvreur},
  ///   title         = {Un point de vue symbolique sur la logique temporelle
  ///                   lin{\'e}aire},
  ///   booktitle     = {Actes du Colloque LaCIM 2000},
  ///   month         = {August},
  ///   year          = {2000},
  ///   pages         = {131--140},
  ///   volume        = {27},
  ///   series        = {Publications du LaCIM},
  ///   publisher     = {Universit{\'e} du Qu{\'e}bec {\`a} Montr{\'e}al},
  ///   editor        = {Pierre Leroux}
  /// }
  /// \endverbatim
  tgba_bdd_concrete* ltl_to_tgba_lacim(const ltl::formula* f, bdd_dict* dict);
}

#endif // SPOT_TGBA_LTL2TGBA_HH
