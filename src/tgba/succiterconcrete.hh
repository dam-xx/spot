#ifndef SPOT_TGBA_SUCCITERCONCRETE_HH
# define SPOT_TGBA_SUCCITERCONCRETE_HH

#include "succiter.hh"
#include "tgbabddcoredata.hh"

namespace spot
{
  class tgba_succ_iterator_concrete: public tgba_succ_iterator
  {
  public:
    tgba_succ_iterator_concrete(const tgba_bdd_core_data& d, bdd successors);
    virtual ~tgba_succ_iterator_concrete();
  
    // iteration
    void first();
    void next();
    bool done();
  
    // inspection
    state_bdd current_state();
    bdd current_condition();
    bdd current_promise();
  
  private:
    const tgba_bdd_core_data& data_;
    bdd succ_set_;		// The set of successors.
    bdd next_succ_set_;		// Unexplored successors (including current_).
    bdd current_;		// Current successor.
  };
}

#endif // SPOT_TGBA_SUCCITERCONCRETE_HH
