#include "formula.hh"

namespace spot
{
  namespace ltl
  {
    formula*
    formula::ref()
    {
      ref_();
      return this;
    }

    formula::~formula()
    {
    }

    void
    formula::unref(formula* f)
    {
      if (f->unref_())
	delete f;
    }

    void
    formula::ref_()
    {
      // Not reference counted by default.
    }

    bool
    formula::unref_()
    {
      // Not reference counted by default.
      return false;
    }
  }
}
