#ifndef SPOT_TGBA_SUCCITER_H
# define SPOT_TGBA_SUCCITER_H

#include "state.hh"

namespace spot
{

  class tgba_succ_iterator
  {
  public:
    virtual
    ~tgba_succ_iterator()
    {
    }

    // iteration
    virtual void first() = 0;
    virtual void next() = 0;
    virtual bool done() = 0;

    // inspection
    virtual state* current_state() = 0;
    virtual bdd current_condition() = 0;
    virtual bdd current_promise() = 0;
  };

}


#endif // SPOT_TGBA_SUCCITER_H
