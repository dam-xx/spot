#ifndef SPOT_TGBA_TGBACOMPLEMENT_HH
# define SPOT_TGBA_TGBACOMPLEMENT_HH

# include "tgba/tgba.hh"

namespace spot
{
  struct safra_tree_automaton;

  /// \brief Build a complemented automaton.
  /// \ingroup tgba
  ///
  /// It creates an automaton that recognizes the
  /// negated language of \a aut.
  ///
  /// 1. First Safra construction algorithm produces a
  ///    deterministic Rabin automaton.
  /// 2. Interpreting this deterministic Rabin automaton as a
  ///    deterministic Streett will produce a complemented automaton.
  /// 3. Then we use a transformation from deterministic Streett
  ///    automaton to nondeterministic Büchi automaton.
  ///
  ///  Safra construction is done in \a tgba_complement, the transformation
  ///  is done on-the-fly when successors are called.
  ///
  /// \sa safra_determinisation, tgba_complement::succ_iter.
  class tgba_complement : public tgba
  {
  public:
    tgba_complement(const tgba* a);
    virtual ~tgba_complement();

    safra_tree_automaton* get_safra() const
    {
      return safra_;
    }

    // tgba interface.
    virtual state* get_init_state() const;
    virtual tgba_succ_iterator*
    succ_iter(const state* local_state,
	      const state* global_state = 0,
	      const tgba* global_automaton = 0) const;

    virtual bdd_dict* get_dict() const;
    virtual std::string format_state(const state* state) const;
    virtual bdd all_acceptance_conditions() const;
    virtual bdd neg_acceptance_conditions() const;

  protected:
    virtual bdd compute_support_conditions(const state* state) const;
    virtual bdd compute_support_variables(const state* state) const;
  private:
    const tgba* automaton_;
    safra_tree_automaton* safra_;
    bdd the_acceptance_cond_;
  };

  /// \brief Produce a dot output of the Safra automaton associated
  /// to \a a.
  ///
  /// @param a The \c tgba_complement with an intermediate Safra
  /// automaton to display
  ///
  void display_safra(const tgba_complement* a);
}

#endif  // SPOT_TGBA_TGBACOMPLEMENT_HH
