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
    /// \param successors The set of successors with ingoing conditions
    /// and promises, represented as a BDD.  The job of this iterator
    /// will be to enumerate the satisfactions of that BDD and split
    /// them into destination state, conditions, and promises.
    /// \param d The core data of the automata.  These contains
    /// sets of variables useful to split a BDD.
    tgba_succ_iterator_concrete(const tgba_bdd_core_data& d, bdd successors);
    virtual ~tgba_succ_iterator_concrete();

    // iteration
    void first();
    void next();
    bool done();

    // inspection
    state_bdd* current_state();
    bdd current_condition();
    bdd current_promise();

  private:
    const tgba_bdd_core_data& data_; ///< Core data of the automata.
    bdd succ_set_;	///< The set of successors.
    bdd next_succ_set_;	///< Unexplored successors (including current_).
    bdd current_;	///< Current successor.
  };
}

#endif // SPOT_TGBA_SUCCITERCONCRETE_HH
