#ifndef SPOT_TGBA_BDDFACTORY_HH
# define SPOT_TGBA_BDDFACTORY_HH

#include <bdd.h>

namespace spot
{

  /// \brief Help construct BDDs by reusing existing variables.
  ///
  /// Spot uses BuDDy as BDD library, and BuDDy needs to
  /// know the number of BDD variables in use.  As a rule
  /// of thumb: the more BDD variables the slowere.  So
  /// creating new BDD variables each time we build a new
  /// Automata won't do.
  ///
  /// To avoid variable explosion, we reuse existing variables
  /// when building new automata, and only allocate new variables
  /// when we ran out of existing variables.  This class helps
  /// reusing variables this way.
  ///
  /// Using the same variables in different automata is not a
  /// problem until an operation is performed on both automata.
  /// In this case some homogenization of BDD variable will be
  /// required.  Such operation is out of the scope of this class.
  class bdd_factory
  {
  public:
    bdd_factory();

    /// \brief Allocate a BDD variable.
    ///
    /// The returned integer can be converted to a BDD using ithvar().
    int create_node();

    /// \brief Allocate a pair BDD variable.
    ///
    /// The returned integer is the first variable of the pair,
    /// it's successor (i.e., n + 1) is the second variable.
    /// They can be converted to BDDs using ithvar().
    int create_pair();

    /// Convert a variable number into a BDD.
    static bdd
    ithvar(int i)
    {
      return bdd_ithvar(i);
    }

  protected:
    static void initialize();	///< Initialize the BDD library.
    int create_nodes(int i);    ///< Create \a i variables.

    static bool initialized; ///< Whether the BDD library has been initialized.
    static int varnum; ///< number of variable in use in the BDD library.
    int varused;  ///< Number of variables used for this construction.
  };
}

#endif // SPOT_TGBA_BDDFACTORY_HH
