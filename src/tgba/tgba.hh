#ifndef SPOT_TGBA_TGBA_HH
# define SPOT_TGBA_TGBA_HH

#include "state.hh"
#include "succiter.hh"
#include "bdddict.hh"

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
  protected:
    tgba();

  public:
    virtual ~tgba();

    /// \brief Get the initial state of the automaton.
    ///
    /// The state has been allocated with \c new.  It is the
    /// responsability of the caller to \c delete it when no
    /// longer needed.
    virtual state* get_init_state() const = 0;

    /// \brief Get an iterator over the successors of \a local_state.
    ///
    /// The iterator has been allocated with \c new.  It is the
    /// responsability of the caller to \c delete it when no
    /// longer needed.
    ///
    /// During synchornized products, additional informations are
    /// passed about the entire product and its state.  Recall that
    /// products can be nested, forming a tree of spot::tgba where
    /// most values are computed on demand.  \a global_automaton
    /// designate the root spot::tgba, and \a global_state its
    /// state.  This two objects can be used by succ_iter() to
    /// restrict the set of successors to compute.
    ///
    /// \param local_state The state whose successors are to be explored.
    /// This pointer is not adopted in any way by \c succ_iter, and
    /// it is still the caller's responsability to delete it when
    /// appropriate (this can be done during the lifetime of
    /// the iterator).
    /// \param global_state In a product, the state of the global
    /// product automaton.  Otherwise, 0.  Like \a locale_state,
    /// \a global_state is not adopted by \c succ_iter.
    /// \param global_automaton In a product, the state of the global
    /// product automaton.  Otherwise, 0.
    virtual tgba_succ_iterator*
    succ_iter(const state* local_state,
	      const state* global_state = 0,
	      const tgba* global_automaton = 0) const = 0;

    /// \brief Get a formula that must hold whatever successor is taken.
    ///
    /// \return A formula which must be verified for all successors
    ///  of \a state.
    ///
    /// This can be as simple as \c bddtrue, or more completely
    /// the disjunction of the condition of all successors.  This
    /// is used as an hint by \c succ_iter() to reduce the number
    /// of successor to compute in a product.
    ///
    /// Sub classes should implement compute_support_conditions(),
    /// this function is just a wrapper that will cache the
    /// last return value for efficiency.
    bdd support_conditions(const state* state) const;

    /// \brief Get the conjunctions of variables tested by
    ///        the outgoing transitions of \a state.
    ///
    /// All variables tested by outgoing transitions must be
    /// returned.  This is mandatory.
    ///
    /// This is used as an hint by some \c succ_iter() to reduce the
    /// number of successor to compute in a product.
    ///
    /// Sub classes should implement compute_support_variables(),
    /// this function is just a wrapper that will cache the
    /// last return value for efficiency.
    bdd support_variables(const state* state) const;

    /// \brief Get the dictionary associated to the automaton.
    ///
    /// State are represented as BDDs.  The dictionary allows
    /// to map BDD variables back to formulae, and vice versa.
    /// This is useful when dealing with several automata (which
    /// may use the same BDD variable for different formula),
    /// or simply when printing.
    virtual bdd_dict* get_dict() const = 0;

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
    /// This is useful when making products: each operand's condition
    /// set should be augmented with the neg_accepting_conditions() of
    /// the other operand.
    virtual bdd neg_accepting_conditions() const = 0;

  protected:
    /// Do the actual computation of tgba::support_conditions().
    virtual bdd compute_support_conditions(const state* state) const = 0;
    /// Do the actual computation of tgba::support_variables().
    virtual bdd compute_support_variables(const state* state) const = 0;
  private:
    mutable const state* last_support_conditions_input_;
    mutable bdd last_support_conditions_output_;
    mutable const state* last_support_variables_input_;
    mutable bdd last_support_variables_output_;
  };

}

#endif // SPOT_TGBA_TGBA_HH
