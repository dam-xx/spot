#ifndef SPOT_TGBA_TGBABDDPROD_HH
# define SPOT_TGBA_TGBABDDPROD_HH

#include "tgba.hh"
#include "statebdd.hh"

namespace spot
{

  /// \brief A state for spot::tgba_bdd_product.  
  ///
  /// This state is in fact a pair of state: the state from the left
  /// automaton and that of the right.
  class state_bdd_product : public state_bdd
  {
  public:
    /// \brief Constructor
    /// \param left The state from the left automaton.
    /// \param right The state from the right automaton.
    /// These state are acquired by spot::state_bdd_product, and will
    /// be deleted on destruction.
    state_bdd_product(state_bdd* left, state_bdd* right)
      : state_bdd(left->as_bdd() & right->as_bdd()), 
	left_(left), 
	right_(right)
    {
    }
    
    virtual ~state_bdd_product();

    state_bdd* 
    left() const
    {
      return left_;
    }

    state_bdd* 
    right() const
    {
      return right_;
    }

    virtual int compare(const state* other) const;

  private:
    state_bdd* left_;		///< State from the left automaton.
    state_bdd* right_;		///< State from the right automaton.
  };


  /// \brief Iterate over the successors of a product computed on the fly.
  class tgba_bdd_product_succ_iterator: public tgba_succ_iterator
  {
  public:
    tgba_bdd_product_succ_iterator(tgba_succ_iterator* left, 
				   tgba_succ_iterator* right);
    
    // iteration
    void first();
    void next();
    bool done();

    // inspection
    state_bdd* current_state();
    bdd current_condition();
    bdd current_promise();

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
  };
  
  /// \brief A lazy product.  (States are computed on the fly.)
  class tgba_bdd_product : public tgba
  {
  public:
    /// \brief Constructor.
    /// \param left The left automata in the product.
    /// \param right The right automata in the product.
    /// Do not be fooled by these arguments: a product \emph is commutative.
    tgba_bdd_product(const tgba& left, const tgba& right);
    
    virtual ~tgba_bdd_product();
    
    virtual state* get_init_state() const;

    virtual tgba_bdd_product_succ_iterator* 
    succ_iter(const state* state) const;

    virtual const tgba_bdd_dict& get_dict() const;

    virtual std::string format_state(const state* state) const;

  private:
    const tgba* left_;
    bool left_should_be_freed_;
    const tgba* right_;
    bool right_should_be_freed_;
    tgba_bdd_dict dict_;
  };

}

#endif // SPOT_TGBA_TGBABDDPROD_HH
