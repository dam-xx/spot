#ifndef SPOT_TGBA_TGBA_HH
# define SPOT_TGBA_TGBA_HH

#include "state.hh"
#include "succiter.hh"
#include "tgbabdddict.hh"

namespace spot
{
  /// \brief A Transition-based Generalized Büchi Automaton.
  ///
  /// The acronym TGBA (Transition-based Generalized Büchi Automaton)
  /// was coined by Dimitra Giannakopoulou and Flavio Lerda
  /// in "From States to Transitions: Improving Translation of LTL
  /// Formulae to Büchi Automata".  (FORTE'02)
  ///
  /// TGBAs are transition-based, meanings their labels are put
  /// on arcs, not on nodes.  They use Generalized Büchi acceptance
  /// conditions: there are several accepting sets (of
  /// transitions), and a path can be accepted only if it traverse
  /// at least one transition of each set infinitely often.
  ///
  /// Browsing such automaton can be achieved using two functions.
  /// \c get_init_state, and \c succ_iter.  The former returns
  /// the initial state while the latter allows to explore the
  /// successor states of any state.
  ///
  /// Note that although this is a transition-based automata,
  /// we never represent transitions!  Transition informations are
  /// obtained by querying the iterator over the successors of
  /// a state.
  class tgba
  {
  public:
    virtual
    ~tgba()
    {
    }

    /// \brief Get the initial state of the automaton.
    ///
    /// The state has been allocated with \c new.  It is the
    /// responsability of the caller to \c delete it when no
    /// longer needed.
    virtual state* get_init_state() const = 0;

    /// \brief Get an iterator over the successors of \a state.
    ///
    /// The iterator has been allocated with \c new.  It is the
    /// responsability of the caller to \c delete it when no
    /// longer needed.
    ///
    /// \param state is the state whose successors are to be explored.
    /// This pointer is not adopted in any way by \c succ_iter, and
    /// it is still the caller's responsability to delete it when
    /// appropriate (this can be done during the lifetime of
    /// the iterator).
    virtual tgba_succ_iterator* succ_iter(const state* state) const = 0;

    /// \brief Get the dictionary associated to the automaton.
    ///
    /// State are represented as BDDs.  The dictionary allows
    /// to map BDD variables back to formulae, and vice versa.
    /// This is useful when dealing with several automata (which
    /// may use the same BDD variable for different formula),
    /// or simply when printing.
    virtual const tgba_bdd_dict& get_dict() const = 0;

    /// \brief Format the state as a string for printing.
    ///
    /// This formating is the responsability of the automata
    /// who owns the state.
    virtual std::string format_state(const state* state) const = 0;

    /// \brief Return the set of all accepting conditions used
    /// by this automaton.
    ///
    /// The goal of the emptiness check is to ensure that
    /// a strongly connected component walks through each
    /// of these acceptiong conditions.  I.e., the union
    /// of the acceptiong conditions of all transition in
    /// the SCC should be equal to the result of this function.
    virtual bdd all_accepting_conditions() const = 0;

    /// \brief Return the conjuction of all negated accepting
    /// variables.
    ///
    /// For instance if the automaton uses variables <tt>Acc[a]</tt>,
    /// <tt>Acc[b]</tt> and <tt>Acc[c]</tt> to describe accepting sets,
    /// this function should return <tt>!Acc[a]\&!Acc[b]\&!Acc[c]</tt>.
    ///
    /// This is useful when making products: each operand conditions
    /// set should be augmented with the neg_accepting_conditions() of
    /// the other operand.
    virtual bdd neg_accepting_conditions() const = 0;
  };

}

#endif // SPOT_TGBA_TGBA_HH
