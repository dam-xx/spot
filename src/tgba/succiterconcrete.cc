#include "succiterconcrete.hh"

namespace spot
{
  tgba_succ_iterator_concrete::tgba_succ_iterator_concrete
  (const tgba_bdd_core_data& d, bdd successors)
    : data_(d), succ_set_(successors), next_succ_set_(successors),
      current_(bddfalse)
  {
  }
  
  tgba_succ_iterator_concrete::~tgba_succ_iterator_concrete()
  {
  }
  
  void
  tgba_succ_iterator_concrete::first()
  {
    next_succ_set_ = succ_set_;
    if (!done())
      next();
  }
  
  void
  tgba_succ_iterator_concrete::next()
  {
    assert(!done());
  
    // FIXME: Iterating on the successors this way (calling bdd_satone
    // and NANDing out the result from the set) requires several descent
    // of the BDD.  Maybe it would be faster to compute all satisfying
    // formula in one operation.
    next_succ_set_ &= !current_;
    current_ = bdd_satone(next_succ_set_);
  }
  
  bool
  tgba_succ_iterator_concrete::done()
  {
    return next_succ_set_ == bddfalse;
  }
  
  state_bdd
  tgba_succ_iterator_concrete::current_state()
  {
    assert(!done());
    return bdd_exist(current_, data_.notnow_set);
  }
  
  bdd
  tgba_succ_iterator_concrete::current_condition()
  {
    assert(!done());
    return bdd_exist(current_, data_.notvar_set);
  }
  
  bdd
  tgba_succ_iterator_concrete::current_promise()
  {
    assert(!done());
    return bdd_exist(current_, data_.notprom_set);
  }

}
