#ifndef SPOT_TGBA_BDDPRINT_HH
# define SPOT_TGBA_BDDPRINT_HH

#include <string>
#include <iostream>
#include "tgbabdddict.hh"
#include <bdd.h>

namespace spot
{

  std::ostream& bdd_print_set(std::ostream& os,
			      const tgba_bdd_dict& dict, bdd b);
  std::string bdd_format_set(const tgba_bdd_dict& dict, bdd b);

  std::ostream& bdd_print_dot(std::ostream& os,
			      const tgba_bdd_dict& dict, bdd b);

  std::ostream& bdd_print_table(std::ostream& os,
				const tgba_bdd_dict& dict, bdd b);

}

#endif // SPOT_TGBA_BDDPRINT_HH
