#ifndef SPOT_MISC_BDDLT_HH
# define SPOT_MISC_BDDLT_HH

# include <bdd.h>

namespace spot
{
  /// Comparison functor for BDDs.
  struct bdd_less_than
  {
    bool
    operator()(const bdd& left, const bdd& right) const
    {
      return left.id() < right.id();
    }
  };
}

#endif // SPOT_MISC_BDDLT_HH
