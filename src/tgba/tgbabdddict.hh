#ifndef SPOT_TGBA_TGBABDDDICT_H
# define SPOT_TGBA_TGBABDDDICT_H

#include <map>
#include <iostream>
#include "ltlast/formula.hh"

namespace spot
{

  /// Dictionary of BDD variables.
  struct tgba_bdd_dict
  {
    /// Formula-to-BDD-variable maps.
    typedef std::map<const ltl::formula*, int> fv_map;
    /// BDD-variable-to-formula maps.
    typedef std::map<int, const ltl::formula*> vf_map;

    fv_map now_map;		///< Maps formulae to "Now" BDD variables.
    vf_map now_formula_map;	///< Maps "Now" BDD variables to formulae.
    fv_map var_map;		///< Maps atomic propisitions to BDD variables
    vf_map var_formula_map;     ///< Maps BDD variables to atomic propisitions
    fv_map prom_map;		///< Maps promises to BDD variables.
    vf_map prom_formula_map;    ///< Maps BDD variables to promises.

    /// \brief Dump all variables for debugging.
    /// \param os The output stream.
    std::ostream& dump(std::ostream& os) const;
  };
}

#endif // SPOT_TGBA_TGBABDDDICT_H
