#include "bddfactory.hh"

namespace spot
{
  bdd_factory::bdd_factory()
    : varused(0)
  {
    initialize();
  }
  
  int
  bdd_factory::create_node()
  {
    return create_nodes(1);
  }
  
  int
  bdd_factory::create_pair()
  {
    return create_nodes(2);
  }
  
  int
  bdd_factory::create_nodes(int i)
  {
    int res = varused;
    varused += i;
    if (varnum < varused)
      {
        bdd_extvarnum(varused - varnum);
        varnum = varused;
      }
    return res;
  }

  void
  bdd_factory::initialize()
  {
    if (initialized)
      return;
    initialized = true;
    bdd_init(50000, 5000);
    bdd_setvarnum(varnum);
  }  
  
  bool bdd_factory::initialized = false;
  int bdd_factory::varnum = 2;
}
