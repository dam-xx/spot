#include "refformula.hh"
#include <cassert>

namespace spot
{
  namespace ltl
  {
    ref_formula::ref_formula()
      : ref_count_(0)
    {
    }

    ref_formula::~ref_formula()
    {
    }

    void
    ref_formula::ref_()
    {
      ++ref_count_;
    }

    bool
    ref_formula::unref_()
    {
      assert(ref_count_ > 0);
      return !--ref_count_;
    }

  }
}
