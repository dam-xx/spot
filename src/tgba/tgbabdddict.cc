#include "tgbabdddict.hh"
#include "ltlvisit/tostring.hh"
#include "ltlvisit/clone.hh"
#include "ltlvisit/destroy.hh"

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
    os << "Accepting Conditions:" << std::endl;
    for (sii = acc_map.begin(); sii != acc_map.end(); ++sii)
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
    for (i = other.acc_map.begin(); i != other.acc_map.end(); ++i)
      {
	fv_map::const_iterator i2 = acc_map.find(i->first);
	if (i2 == acc_map.end() || i->second != i2->second)
	  return false;
      }
    return true;
  }

  tgba_bdd_dict::tgba_bdd_dict()
  {
  }

  tgba_bdd_dict::tgba_bdd_dict(const tgba_bdd_dict& other)
    : now_map(other.now_map),
      now_formula_map(other.now_formula_map),
      var_map(other.var_map),
      var_formula_map(other.var_formula_map),
      acc_map(other.acc_map),
      acc_formula_map(other.acc_formula_map)
  {
    fv_map::iterator i;
    for (i = now_map.begin(); i != now_map.end(); ++i)
      ltl::clone(i->first);
    for (i = var_map.begin(); i != var_map.end(); ++i)
      ltl::clone(i->first);
    for (i = acc_map.begin(); i != acc_map.end(); ++i)
      ltl::clone(i->first);
  }

  tgba_bdd_dict&
  tgba_bdd_dict::operator=(const tgba_bdd_dict& other)
  {
    if (this != &other)
      {
	this->~tgba_bdd_dict();
	new (this) tgba_bdd_dict(other);
      }
    return *this;
  }

  tgba_bdd_dict::~tgba_bdd_dict()
  {
    fv_map::iterator i;
    for (i = now_map.begin(); i != now_map.end(); ++i)
      ltl::destroy(i->first);
    for (i = var_map.begin(); i != var_map.end(); ++i)
      ltl::destroy(i->first);
    for (i = acc_map.begin(); i != acc_map.end(); ++i)
      ltl::destroy(i->first);
  }

}
