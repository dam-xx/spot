#ifndef SPOT_TGBA_TGBACOMPLEMENT_HH
#define SPOT_TGBA_TGBACOMPLEMENT_HH

#include <vector>
#include "bdd.h"
#include "tgba.hh"
#include "tgba/tgbasgba.hh"

namespace spot
{
  class bdd_ordered
  {
  public:
    bdd_ordered()
      : order_(0)
    {};

    bdd_ordered(int bdd_, unsigned order_)
      : bdd_(bdd_), order_(order_)
    {
    }

    unsigned order() const
    {
      return order_;
    }

    unsigned& order()
    {
      return order_;
    }

    bdd get_bdd() const
    {
      return bdd_ithvar(bdd_);
    }
  private:
    int bdd_;
    unsigned order_;
  };

  typedef std::vector<bdd_ordered> acc_list_t;

  /// \brief Build a complemented automaton.
  /// \ingroup tgba
  ///
  /// The construction comes from:
  /// @Article{         kupferman.05.tcs,
  ///   title           = {{From complementation to certification}},
  ///   author          = {Kupferman, O. and Vardi, M.Y.},
  ///   journal         = {Theoretical Computer Science},
  ///   volume  	= {345},
  ///   number          = {1},
  ///   pages		= {83--100},
  ///   year		= {2005},
  ///   publisher	= {Elsevier}
  /// }
  ///
  /// The original automaton is used as a States-based Generalized
  /// Büchi Automaton.
  ///
  /// The construction is done on-the-fly, by the
  /// \c tgba_complement_succ_iterator class.
  /// \see tgba_complement_succ_iteratora
  class tgba_complement : public tgba
  {
  public:
    tgba_complement(const tgba* a);
    virtual ~tgba_complement();

    // tgba interface
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
    /// Retrieve all the atomic acceptance conditions of the automaton.
    /// They are inserted into \a acc_list_.
    void get_acc_list();
  private:
    const tgba_sgba_proxy* automaton_;
    bdd the_acceptance_cond_;
    unsigned nb_states_;
    acc_list_t acc_list_;
  }; // end class tgba_complement.

} // end namespace spot.


#endif  // !SPOT_TGBA_TGBACOMPLEMENT_HH
