#ifndef SPOT_TGBA_BDDFACTORY_HH
# define SPOT_TGBA_BDDFACTORY_HH

#include <bdd.h>

namespace spot
{

  class bdd_factory
  {
  public:
    bdd_factory();

    int create_node();
    int create_pair();
  
    static bdd
    ithvar(int i)
    {
      return bdd_ithvar(i);
    }
  
  protected:
    static void initialize();
    int create_nodes(int i);
  
    static bool initialized;
    static int varnum;
    int varused;
  };
}

#endif // SPOT_TGBA_BDDFACTORY_HH
