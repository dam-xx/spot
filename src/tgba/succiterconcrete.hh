#ifndef SPOT_TGBA_SUCCITERCONCRETE_HH
# define SPOT_TGBA_SUCCITERCONCRETE_HH

#include "statebdd.hh"
#include "succiter.hh"
#include "tgbabddcoredata.hh"

namespace spot
{
  /// A concrete iterator over successors of a TGBA state.
  class tgba_succ_iterator_concrete: public tgba_succ_iterator
  {
  public:
    /// \brief Build a spot::tgba_succ_iterator_concrete.
    ///
    /// \param successors The set of successors with ingoing
    ///   conditions and accepting conditions, represented as a BDD.
    ///   The job of this iterator will be to enumerate the
    ///   satisfactions of that BDD and split them into destination
    ///   states and conditions, and compute accepting conditions.
    /// \param d The core data of the automata.
    ///   These contains sets of variables useful to split a BDD, and
    ///   compute accepting conditions.
    tgba_succ_iterator_concrete(const tgba_bdd_core_data& d, bdd successors);
    virtual ~tgba_succ_iterator_concrete();

    // iteration
    void first();
    void next();
    bool done() const;

    // inspection
    state_bdd* current_state();
    bdd current_condition();
    bdd current_accepting_conditions();

  private:
    const tgba_bdd_core_data& data_; ///< Core data of the automaton.
    bdd succ_set_;	///< The set of successors.
    bdd succ_set_left_;	///< Unexplored successors (including current_).
    bdd trans_dest_;	///< Destination state of currently explored subset
    bdd trans_set_;     ///< Set of successors with the same destination.
    bdd neg_trans_set_;	///< Negation of trans_set_.
    bdd trans_set_left_;///< Part of trans_set_ not yet explored.
    bdd current_;	///< \brief Current successor, as a conjunction of
			///         atomic proposition and Next variables.
    bdd current_state_;	///< \brief Current successor, as a
			///         conjunction of Now variables.
  };
}

#endif // SPOT_TGBA_SUCCITERCONCRETE_HH
