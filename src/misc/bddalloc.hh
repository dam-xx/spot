#ifndef SPOT_MISC_BDDALLOC_HH
# define SPOT_MISC_BDDALLOC_HH

#include <list>
#include <utility>

namespace spot
{

  /// Manage ranges of variables.
  class bdd_allocator
  {
  protected:
    /// Default constructor.
    bdd_allocator();
    /// Initialize the BDD library.
    static void initialize();
    /// Allocate \a n BDD variables.
    int allocate_variables(int n);
    /// Release \a n BDD variables starting at \a base.
    void release_variables(int base, int n);

    static bool initialized; ///< Whether the BDD library has been initialized.
    static int varnum; ///< number of variables in use in the BDD library.
    int lvarnum; ///< number of variables in use in this allocator.
    typedef std::pair<int, int> pos_lenght_pair;
    typedef std::list<pos_lenght_pair> free_list_type;
    free_list_type free_list; ///< Tracks unused BDD variables.
  private:
    /// Require more variables.
    void extvarnum(int more);
  };

}

#endif // SPOT_MISC_BDDALLOC_HH
