#include <cassert>
#include "tgbabddconcreteproduct.hh"

namespace spot
{
  /// \brief Helper class for product().
  ///
  /// As both automata are encoded using BDD, we just have
  /// to homogenize the variable numbers before ANDing the
  /// relations and initial states.
  class tgba_bdd_product_factory: public tgba_bdd_factory
  {
  public:
    tgba_bdd_product_factory(const tgba_bdd_concrete* left,
			     const tgba_bdd_concrete* right)
      : dict_(left->get_dict()),
	left_(left),
	right_(right),
	data_(left_->get_core_data(), right_->get_core_data()),
	init_(left_->get_init_bdd() & right_->get_init_bdd())
    {
      assert(dict_ == right->get_dict());
    }

    virtual
    ~tgba_bdd_product_factory()
    {
    }

    const tgba_bdd_core_data&
    get_core_data() const
    {
      return data_;
    }

    bdd_dict*
    get_dict() const
    {
      return dict_;
    }

    bdd
    get_init_state() const
    {
      return init_;
    }

  private:
    bdd_dict* dict_;
    const tgba_bdd_concrete* left_;
    const tgba_bdd_concrete* right_;
    tgba_bdd_core_data data_;
    bdd init_;
  };

  tgba_bdd_concrete*
  product(const tgba_bdd_concrete* left, const tgba_bdd_concrete* right)
  {
    tgba_bdd_product_factory p(left, right);
    return new tgba_bdd_concrete(p, p.get_init_state());
  }
}
