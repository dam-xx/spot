#ifndef SPOT_MISC_MINATO_HH
# define SPOT_MISC_MINATO_HH

# include <bdd.h>
# include <stack>

namespace spot
{
  /// \brief Generate an irredundant sum-of-products (ISOP) form of a
  /// BDD function.
  ///
  /// This algorithm implements a derecursived version the Minato-Morreale
  /// algorithm presented in the following paper.
  /// \verbatim
  /// @InProceedings{   minato.92.sasimi,
  ///   author        = {Shin-ichi Minato},
  ///   title         = {Fast Generation of Irredundant Sum-of-Products Forms
  ///                   from Binary Decision Diagrams},
  ///   booktitle     = {Proceedings of the third Synthesis and Simulation
  ///                   and Meeting International Interchange workshop
  ///                   (SASIMI'92)},
  ///   pages         = {64--73},
  ///   year          = {1992},
  ///   address       = {Kobe, Japan},
  ///   month         = {April}
  /// }
  /// \endverbatim
  class minato_isop
  {
  public:
    /// \brief Conctructor.
    /// \arg input The BDD function to translate in ISOP.
    minato_isop(bdd input);
    /// \brief Compute the next sum term of the ISOP form.
    /// Return \c bddfalse when all terms have been output.
    bdd next();

  private:
    /// Internal variables for minato_isop.
    struct local_vars
    {
      // If you are following the paper, f_min and f_max corresponds
      // to the pair of BDD function used to encode the ternary function `f'
      // (see Section 3.4).
      // Also note that f0, f0', and f0'' all share the same _max function.
      // Likewise for f1, f1', and f1''.
      bdd f_min, f_max;
      // Because we need a non-recursive version of the algorithm,
      // we had to split it in four step (each step is separated
      // from the other by a call to ISOP in the original algorithm).
      enum { FirstStep, SecondStep, ThirdStep, FourthStep } step;
      bdd v1;
      bdd f0_min, f0_max;
      bdd f1_min, f1_max;
      bdd g0, g1;
      local_vars(bdd f_min, bdd f_max)
	: f_min(f_min), f_max(f_max), step(FirstStep) {}
    };
    std::stack<local_vars> todo;
    std::stack<bdd> cube;
    bdd ret;
  };
}

#endif // SPOT_MISC_MINATO_HH
