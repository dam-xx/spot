#include "tgbabddconcreteproduct.hh"
#include "tgbabddtranslatefactory.hh"
#include "dictunion.hh"

namespace spot
{
  class tgba_bdd_product_factory: public tgba_bdd_factory
  {
  public:
    tgba_bdd_product_factory(const tgba_bdd_concrete& left,
			     const tgba_bdd_concrete& right)
      : dict_(tgba_bdd_dict_union(left.get_dict(),
				  right.get_dict())),
        fact_left_(left, dict_),
        fact_right_(right, dict_),
        data_(fact_left_.get_core_data(), fact_right_.get_core_data()),
        init_(fact_left_.get_init_state() & fact_right_.get_init_state())
    {
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
  
    const tgba_bdd_dict&
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
    tgba_bdd_dict dict_;
    tgba_bdd_translate_factory fact_left_;
    tgba_bdd_translate_factory fact_right_;
    tgba_bdd_core_data data_;
    bdd init_;
  };
  
  tgba_bdd_concrete
  product(const tgba_bdd_concrete& left, const tgba_bdd_concrete& right)
  {
  
    tgba_bdd_product_factory p(left, right);
    return tgba_bdd_concrete(p, p.get_init_state());
  }
}
