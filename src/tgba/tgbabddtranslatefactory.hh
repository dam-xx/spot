#ifndef SPOT_TGBA_TGBABDDTRANSLATEFACTORY_HH
# define SPOT_TGBA_TGBABDDTRANSLATEFACTORY_HH

#include "tgbabddfactory.hh"
#include "tgbabddconcrete.hh"

namespace spot
{

  /// A spot::tgba_bdd_factory than renumber BDD variables.
  class tgba_bdd_translate_factory: public tgba_bdd_factory
  {
  public:
    ///\brief Construct a spot::tgba_bdd_translate_factory
    ///
    /// \param from The Automata to copy.
    /// \param to The dictionary of variable number to use.
    tgba_bdd_translate_factory(const tgba_bdd_concrete& from,
			       const tgba_bdd_dict& to);

    virtual ~tgba_bdd_translate_factory();

    const tgba_bdd_core_data& get_core_data() const;
    const tgba_bdd_dict& get_dict() const;

    /// Get the new initial state.
    bdd get_init_state() const;

  protected:
    /// Compute renaming pairs.
    bddPair* compute_pairs(const tgba_bdd_dict& from);

  private:
    tgba_bdd_core_data data_;
    tgba_bdd_dict dict_;
    bdd init_;
  };

  /// Reorder the variables of an automata.
  tgba_bdd_concrete defrag(const tgba_bdd_concrete& a);
}

#endif // SPOT_TGBA_TGBABDDTRANSLATEFACTORY_HH
