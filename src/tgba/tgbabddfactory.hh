#ifndef SPOT_TGBA_TGBABDDFACTORY_H
# define SPOT_TGBA_TGBABDDFACTORY_H

#include "tgbabddcoredata.hh"
#include "tgbabdddict.hh"

namespace spot
{
  /// \brief Abstract class for spot::tgba_bdd_concrete factories.
  ///
  /// A spot::tgba_bdd_concrete can be constructed from anything that
  /// supplies core data and their associated dictionary.
  class tgba_bdd_factory
  {
  public:
    /// Get the core data for the new automata.
    virtual const tgba_bdd_core_data& get_core_data() const = 0;
    /// Get the dictionary for the new automata.
    virtual const tgba_bdd_dict& get_dict() const = 0;
  };
}

#endif // SPOT_TGBA_TGBABDDFACTORY_H
