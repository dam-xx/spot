#ifndef SPOT_TGBA_TGBABDDCONCRETE_HH
# define SPOT_TGBA_TGBABDDCONCRETE_HH

#include "tgba.hh"
#include "statebdd.hh"
#include "tgbabddfactory.hh"
#include "succiterconcrete.hh"

namespace spot
{
  /// \brief A concrete spot::tgba implemented using BDDs.
  ///
  class tgba_bdd_concrete: public tgba
  {
  public:
    /// \brief Construct a tgba_bdd_concrete with unknown initial state.
    ///
    /// set_init_state() should be called later.
    tgba_bdd_concrete(const tgba_bdd_factory& fact);

    /// \brief Construct a tgba_bdd_concrete with known initial state.
    tgba_bdd_concrete(const tgba_bdd_factory& fact, bdd init);

    ~tgba_bdd_concrete();

    /// \brief Set the initial state.
    void set_init_state(bdd s);

    state_bdd* get_init_state() const;

    /// \brief Get the initial state directly as a BDD.
    ///
    /// The sole point of this method is to prevent writing
    /// horrors such as
    /// \code
    ///   state_bdd* s = automata.get_init_state();
    ///   some_class some_instance(s->as_bdd());
    ///   delete s;
    /// \endcode
    bdd get_init_bdd() const;

    tgba_succ_iterator_concrete* succ_iter(const state* state) const;

    std::string format_state(const state* state) const;

    const tgba_bdd_dict& get_dict() const;

    /// \brief Get the core data associated to this automaton.
    ///
    /// These data includes the various BDD used to represent
    /// the relation, encode variable sets, Next-to-Now rewrite
    /// rules, etc.
    const tgba_bdd_core_data& get_core_data() const;

  protected:
    tgba_bdd_core_data data_;	///< Core data associated to the automaton.
    tgba_bdd_dict dict_;	///< Dictionary used by the automaton.
    bdd init_;			///< Initial state.
  };
}

#endif // SPOT_TGBA_TGBABDDCONCRETE_HH
