#ifndef SPOT_TGBA_TGBAPRODUCT_HH
# define SPOT_TGBA_TGBAPRODUCT_HH

#include "tgba.hh"
#include "statebdd.hh"

namespace spot
{

  /// \brief A state for spot::tgba_product.
  ///
  /// This state is in fact a pair of state: the state from the left
  /// automaton and that of the right.
  class state_product : public state
  {
  public:
    /// \brief Constructor
    /// \param left The state from the left automaton.
    /// \param right The state from the right automaton.
    /// These states are acquired by spot::state_product, and will
    /// be deleted on destruction.
    state_product(state* left, state* right)
      :	left_(left),
	right_(right)
    {
    }

    /// Copy constructor
    state_product(const state_product& o);

    virtual ~state_product();

    state*
    left() const
    {
      return left_;
    }

    state*
    right() const
    {
      return right_;
    }

    virtual int compare(const state* other) const;
    virtual state_product* clone() const;

  private:
    state* left_;		///< State from the left automaton.
    state* right_;		///< State from the right automaton.
  };


  /// \brief Iterate over the successors of a product computed on the fly.
  class tgba_succ_iterator_product: public tgba_succ_iterator
  {
  public:
    tgba_succ_iterator_product(tgba_succ_iterator* left,
			       tgba_succ_iterator* right,
			       bdd left_neg, bdd right_neg);

    virtual ~tgba_succ_iterator_product();

    // iteration
    void first();
    void next();
    bool done() const;

    // inspection
    state_product* current_state() const;
    bdd current_condition() const;
    bdd current_accepting_conditions() const;

  private:
    //@{
    /// Internal routines to advance to the next successor.
    void step_();
    void next_non_false_();
    //@}

  protected:
    tgba_succ_iterator* left_;
    tgba_succ_iterator* right_;
    bdd current_cond_;
    bdd left_neg_;
    bdd right_neg_;
  };

  /// \brief A lazy product.  (States are computed on the fly.)
  class tgba_product: public tgba
  {
  public:
    /// \brief Constructor.
    /// \param left The left automata in the product.
    /// \param right The right automata in the product.
    /// Do not be fooled by these arguments: a product is commutative.
    tgba_product(const tgba* left, const tgba* right);

    virtual ~tgba_product();

    virtual state* get_init_state() const;

    virtual tgba_succ_iterator_product*
    succ_iter(const state* local_state,
	      const state* global_state = 0,
	      const tgba* global_automaton = 0) const;

    virtual bdd_dict* get_dict() const;

    virtual std::string format_state(const state* state) const;

    virtual state* project_state(const state* s, const tgba* t) const;

    virtual bdd all_accepting_conditions() const;
    virtual bdd neg_accepting_conditions() const;

  protected:
    virtual bdd compute_support_conditions(const state* state) const;
    virtual bdd compute_support_variables(const state* state) const;

  private:
    bdd_dict* dict_;
    const tgba* left_;
    const tgba* right_;
    bdd all_accepting_conditions_;
    bdd neg_accepting_conditions_;
    // Disallow copy.
    tgba_product(const tgba_product&);
    tgba_product& tgba_product::operator=(const tgba_product&);
  };

}

#endif // SPOT_TGBA_TGBAPRODUCT_HH
