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
    /// \c relation uses three kinds of variables:
    /// \li "Now" variables, that encode the current state
    /// \li "Next" variables, that encode the destination state
    /// \li atomic propositions, which are things to verify before going on
    ///     to the next state
    bdd relation;

    /// \brief encodes the accepting conditions
    ///
    /// <tt>a U b</tt>, or <tt>F b</tt>, both imply that \c b should
    /// be verified eventually.  We encode this with generalized Büchi
    /// acceptating conditions.  An accepting set, called
    /// <tt>Acc[b]</tt>, hold all the state that do not promise to
    /// verify \c b eventually.  (I.e., all the states that contain \c
    /// b, or do not contain <tt>a U b</tt>, or <tt>F b</tt>.)
    ///
    /// The spot::succ_iter::current_accepting_conditions() method
    /// will return the \c Acc[x] variables of the accepting sets
    /// in which a transition is.  Actually we never return \c Acc[x]
    /// alone, but \c Acc[x] and all other accepting variables negated.
    ///
    /// So if there is three accepting set \c a, \c b, and \c c, and a
    /// transition is in set \c a, we'll return <tt>
    /// Acc[a]&!Acc[b]&!Acc[c]</tt>. If the transition is in both \c
    /// a and \c b, we'll return <tt>(Acc[a]\&!Acc[b]\&!Acc[c]) \c | \c
    /// (!Acc[a]\&Acc[b]\&!Acc[c])</tt>.
    ///
    /// Accepting conditions are attributed to transitions and are
    /// only concerned by atomic propositions (which label the
    /// transitions) and Next variables (the destination).  Typically,
    /// a transition should bear the variable \c Acc[b] if it doesn't
    /// check for `b' and have a destination of the form <tt>a U b</tt>,
    /// or <tt>F b</tt>.
    ///
    /// To summarize, \c accepting_conditions contains three kinds of
    /// variables:
    /// \li "Next" variables, that encode the destination state,
    /// \li atomic propositions, which are things to verify before going on
    ///     to the next state,
    /// \li "Acc" variables.
    bdd accepting_conditions;

    /// \brief The set of all accepting conditions used by the Automaton.
    ///
    /// The goal of the emptiness check is to ensure that
    /// a strongly connected component walks through each
    /// of these acceptiong conditions.  I.e., the union
    /// of the acceptiong conditions of all transition in
    /// the SCC should be equal to the result of this function.
    bdd all_accepting_conditions;

    /// The conjunction of all Now variables, in their positive form.
    bdd now_set;
    /// The conjunction of all Next variables, in their positive form.
    bdd next_set;
    /// The conjunction of all Now variables, in their negated form.
    bdd negnow_set;
    /// \brief The (positive) conjunction of all variables which are
    /// not Now variables.
    bdd notnow_set;
    /// \brief The (positive) conjunction of all variables which are
    /// not Next variables.
    bdd notnext_set;
    /// \brief The (positive) conjunction of all variables which are
    /// atomic propositions.
    bdd var_set;
    /// \brief The (positive) conjunction of all variables which are
    /// not atomic propositions.
    bdd notvar_set;
    /// \brief The (positive) conjunction of all Next variables
    /// and atomic propositions.
    bdd varandnext_set;
    /// \brief The (positive) conjunction of all variables which are
    /// accepting conditions.
    bdd acc_set;
    /// \brief The (positive) conjunction of all variables which are not
    /// accepting conditions.
    bdd notacc_set;
    /// \brief The negative conjunction of all variables which are accepting
    /// conditions.
    bdd negacc_set;

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
    /// \brief Update the variable sets to take a new accepting condition
    /// into account.
    void declare_accepting_condition(bdd prom);
  };
}

#endif // SPOT_TGBA_TGBABDDCOREDATA_HH
