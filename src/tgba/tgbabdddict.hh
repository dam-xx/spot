#ifndef SPOT_TGBA_TGBABDDDICT_H
# define SPOT_TGBA_TGBABDDDICT_H

#include <map>
#include <iostream>
#include "ltlast/formula.hh"

namespace spot
{
  struct tgba_bdd_dict
  {
    // Dictionaries for BDD variables.

    // formula-to-BDD-variable maps
    typedef std::map<const ltl::formula*, int> fv_map;
    // BDD-variable-to-formula maps
    typedef std::map<int, const ltl::formula*> vf_map;

    fv_map now_map;
    vf_map now_formula_map;
    fv_map var_map;
    vf_map var_formula_map;
    fv_map prom_map;
    vf_map prom_formula_map;

    std::ostream& dump(std::ostream& os) const;
  };
}

#endif // SPOT_TGBA_TGBABDDDICT_H
