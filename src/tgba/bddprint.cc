#include <sstream>
#include <cassert>
#include "bddprint.hh"
#include "ltlvisit/tostring.hh"

namespace spot
{
  /// Global dictionary used by print_handler() to lookup variables.
  static const tgba_bdd_dict* dict;

  /// Global flag to enable Prom[x] output (instead of `x').
  static bool want_prom;

  /// Stream handler used by Buddy to display BDD variables.
  static void
  print_handler(std::ostream& o, int var)
  {
    tgba_bdd_dict::vf_map::const_iterator isi =
      dict->var_formula_map.find(var);
    if (isi != dict->var_formula_map.end())
      to_string(isi->second, o);
    else
      {
	isi = dict->prom_formula_map.find(var);
	if (isi != dict->prom_formula_map.end())
	  {
	    if (want_prom)
	      o << "Prom[";
	    to_string(isi->second, o);
	    if (want_prom)
	      o << "]";
	  }
	else
	  {
	    isi = dict->now_formula_map.find(var);
	    if (isi != dict->now_formula_map.end())
	      {
		o << "Now["; to_string(isi->second, o) << "]";
	      }
	    else
	      {
		isi = dict->now_formula_map.find(var - 1);
		if (isi != dict->now_formula_map.end())
		  {
		    o << "Next["; to_string(isi->second, o) << "]";
		  }
		else
		  {
		    o << "?" << var;
		  }
	      }
	  }
      }
  }


  static std::ostream* where;
  static void
  print_sat_handler(char* varset, int size)
  {
    bool not_first = false;
    for (int v = 0; v < size; ++v)
      {
	if (varset[v] < 0)
	  continue;
	if (not_first)
	  *where << " ";
	else
	  not_first = true;
	if (varset[v] == 0)
	  *where << "!";
	print_handler(*where, v);
      }
  }

  std::ostream&
  bdd_print_sat(std::ostream& os, const tgba_bdd_dict& d, bdd b)
  {
    dict = &d;
    where = &os;
    want_prom = false;
    assert (bdd_satone(b) == b);
    bdd_allsat (b, print_sat_handler);
    return os;
  }

  std::string
  bdd_format_sat(const tgba_bdd_dict& d, bdd b)
  {
    std::ostringstream os;
    bdd_print_sat(os, d, b);
    return os.str();
  }

  std::ostream&
  bdd_print_set(std::ostream& os, const tgba_bdd_dict& d, bdd b)
  {
    dict = &d;
    want_prom = true;
    bdd_strm_hook(print_handler);
    os << bddset << b;
    bdd_strm_hook(0);
    return os;
  }

  std::string
  bdd_format_set(const tgba_bdd_dict& d, bdd b)
  {
    std::ostringstream os;
    bdd_print_set(os, d, b);
    return os.str();
  }

  std::ostream&
  bdd_print_dot(std::ostream& os, const tgba_bdd_dict& d, bdd b)
  {
    dict = &d;
    want_prom = true;
    bdd_strm_hook(print_handler);
    os << bdddot << b;
    bdd_strm_hook(0);
    return os;
  }

  std::ostream&
  bdd_print_table(std::ostream& os, const tgba_bdd_dict& d, bdd b)
  {
    dict = &d;
    want_prom = true;
    bdd_strm_hook(print_handler);
    os << bddtable << b;
    bdd_strm_hook(0);
    return os;
  }

}
