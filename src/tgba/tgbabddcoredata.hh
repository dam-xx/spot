#ifndef SPOT_TGBA_TGBABDDCOREDATA_HH
# define SPOT_TGBA_TGBABDDCOREDATA_HH

#include <bdd.h>

namespace spot
{
  struct tgba_bdd_core_data
  {
    // RELATION encodes the transition relation of the TGBA.
    // It uses four kinds of variables:
    //   - "Now" variables, that encode the current state
    //   - "Next" variables, that encode the destination state
    //   - atomic propositions, which are things to verify before going on
    //                          to the next state
    //   - promises: a U b, or F b, both implie that b should be verified
    //                eventually.  We encode this with Prom[b], and check
    //                that promises are fullfilled in the emptyness check.
    bdd relation;
  
    // The conjunction of all Now variables, in their positive form.
    bdd now_set;
    // The conjunction of all Now variables, in their negated form.
    bdd negnow_set;
    // The (positive) conjunction of all variables which are not Now variables.
    bdd notnow_set;
    // The (positive) conjunction of all variables which are not atomic
    // propositions.
    bdd notvar_set;
    // The (positive) conjunction of all variables which are not promises.
    bdd notprom_set;
  
    // Record pairings between Next and Now variables.
    bddPair* next_to_now;

  
    tgba_bdd_core_data();
    tgba_bdd_core_data(const tgba_bdd_core_data& copy);
  
    // Merge two core_data.
    tgba_bdd_core_data(const tgba_bdd_core_data& left,
		       const tgba_bdd_core_data& right);
  
    const tgba_bdd_core_data& operator= (const tgba_bdd_core_data& copy);
  
    ~tgba_bdd_core_data();
  
    void declare_now_next(bdd now, bdd next);
    void declare_atomic_prop(bdd var);
    void declare_promise(bdd prom);
  };
}

#endif // SPOT_TGBA_TGBABDDCOREDATA_HH
