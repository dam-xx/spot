#include "tgbabdddict.hh"
#include "ltlvisit/tostring.hh"

namespace spot
{
  std::ostream&
  tgba_bdd_dict::dump(std::ostream& os) const
  {
    fv_map::const_iterator sii;
    os << "Atomic Propositions:" << std::endl;
    for (sii = var_map.begin(); sii != var_map.end(); ++sii)
      {
	os << "  " << sii->second << ": ";
	to_string(sii->first, os) << std::endl;
      }
    os << "States:" << std::endl;
    for (sii = now_map.begin(); sii != now_map.end(); ++sii)
      {
	os << "  " << sii->second << ": Now[";
	to_string(sii->first, os) << "]" << std::endl;
	os << "  " << sii->second + 1 << ": Next[";
	to_string(sii->first, os) << "]" << std::endl;
      }
    os << "Promises:" << std::endl;
    for (sii = prom_map.begin(); sii != prom_map.end(); ++sii)
      {
	os << "  " << sii->second << ": ";
	to_string(sii->first, os) << std::endl;
      }
    return os;
  }
}
