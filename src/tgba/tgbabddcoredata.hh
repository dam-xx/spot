#ifndef SPOT_TGBA_TGBABDDCOREDATA_HH
# define SPOT_TGBA_TGBABDDCOREDATA_HH

#include <bdd.h>

namespace spot
{
  /// Core data for a TGBA encoded using BDDs.
  struct tgba_bdd_core_data
  {
    /// \brief encodes the transition relation of the TGBA.
    ///
    /// \c relation uses four kinds of variables:
    /// \li "Now" variables, that encode the current state
    /// \li "Next" variables, that encode the destination state
    /// \li atomic propositions, which are things to verify before going on
    ///     to the next state
    /// \li promises: \c a \c U \c b, or \c F \cb, both imply that \c b
    ///     should be verified eventually.  We encode this with \c Prom[b],
    ///     and check that promises are fullfilled in the emptyness check.
    bdd relation;

    /// The conjunction of all Now variables, in their positive form.
    bdd now_set;
    /// The conjunction of all Now variables, in their negated form.
    bdd negnow_set;
    /// \brief The (positive) conjunction of all variables which are
    /// not Now variables.
    bdd notnow_set;
    /// \brief The (positive) conjunction of all variables which are 
    /// not atomic propositions.
    bdd notvar_set;
    /// The (positive) conjunction of all variables which are not promises.
    bdd notprom_set;

    /// Record pairings between Next and Now variables.
    bddPair* next_to_now;

    /// \brief Default constructor.
    ///
    /// Initially all variable set are empty and the \c relation is true.
    tgba_bdd_core_data();

    /// Copy constructor.
    tgba_bdd_core_data(const tgba_bdd_core_data& copy);

    /// \brief Merge two tgba_bdd_core_data.
    ///
    /// This is used when building a product of two automata.
    tgba_bdd_core_data(const tgba_bdd_core_data& left,
		       const tgba_bdd_core_data& right);

    const tgba_bdd_core_data& operator= (const tgba_bdd_core_data& copy);

    ~tgba_bdd_core_data();

    /// \brief Update the variable sets to take a new pair of variables into
    /// account.
    void declare_now_next(bdd now, bdd next);
    /// \brief Update the variable sets to take a new automic proposition into
    /// account.
    void declare_atomic_prop(bdd var);
    /// Update the variable sets to take a new promise into account.
    void declare_promise(bdd prom);
  };
}

#endif // SPOT_TGBA_TGBABDDCOREDATA_HH
