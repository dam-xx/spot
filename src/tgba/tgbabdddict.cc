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

  bool
  tgba_bdd_dict::contains(const tgba_bdd_dict& other) const
  {
    fv_map::const_iterator i;
    for (i = other.var_map.begin(); i != other.var_map.end(); ++i)
      {
	fv_map::const_iterator i2 = var_map.find(i->first);
	if (i2 == var_map.end() || i->second != i2->second)
	  return false;
      }
    for (i = other.now_map.begin(); i != other.now_map.end(); ++i)
      {
	fv_map::const_iterator i2 = now_map.find(i->first);
	if (i2 == now_map.end() || i->second != i2->second)
	  return false;
      }
    for (i = other.prom_map.begin(); i != other.prom_map.end(); ++i)
      {
	fv_map::const_iterator i2 = prom_map.find(i->first);
	if (i2 == prom_map.end() || i->second != i2->second)
	  return false;
      }
    return true;
  }

}
