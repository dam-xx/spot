#ifndef SPOT_MISC_BDDLT_HH
# define SPOT_MISC_BDDLT_HH

# include <bdd.h>
# include <functional>

namespace spot
{
  /// Comparison functor for BDDs.
  struct bdd_less_than :
    public std::binary_function<const bdd&, const bdd&, bool>
  {
    bool
    operator()(const bdd& left, const bdd& right) const
    {
      return left.id() < right.id();
    }
  };
}

#endif // SPOT_MISC_BDDLT_HH
