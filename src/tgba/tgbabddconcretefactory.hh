#ifndef SPOT_TGBA_TGBABDDCONCRETEFACTORY_HH
# define SPOT_TGBA_TGBABDDCONCRETEFACTORY_HH

#include "ltlast/formula.hh"
#include "bddfactory.hh"
#include "tgbabddfactory.hh"

namespace spot 
{

  class tgba_bdd_concrete_factory: public bdd_factory, public tgba_bdd_factory
  {
  public:
    virtual ~tgba_bdd_concrete_factory();
  
    int create_state(const ltl::formula* f);  
    int create_atomic_prop(const ltl::formula* f);
    int create_promise(const ltl::formula* f);
  
    const tgba_bdd_core_data& get_core_data() const;
    const tgba_bdd_dict& get_dict() const;
  
    void add_relation(bdd new_rel);
  
  private:
    tgba_bdd_core_data data_;
    tgba_bdd_dict dict_;
  };

}
#endif // SPOT_TGBA_TGBABDDCONCRETEFACTORY_HH
