#include "bddprint.hh"
#include "ltlvisit/tostring.hh"

namespace spot
{

  const tgba_bdd_dict* dict;

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
	    o << "Prom["; to_string(isi->second, o) << "]";
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
  

  std::ostream& 
  bdd_print_set(std::ostream& os, const tgba_bdd_dict& d, bdd b)
  {
    dict = &d;
    bdd_strm_hook(print_handler);
    os << bddset << b;
    bdd_strm_hook(0);
    return os;
  }

  std::ostream& 
  bdd_print_dot(std::ostream& os, const tgba_bdd_dict& d, bdd b)
  {
    dict = &d;
    bdd_strm_hook(print_handler);
    os << bdddot << b;
    bdd_strm_hook(0);
    return os;
  }
  
  std::ostream& 
  bdd_print_table(std::ostream& os, const tgba_bdd_dict& d, bdd b)
  {
    dict = &d;
    bdd_strm_hook(print_handler);
    os << bddtable << b;
    bdd_strm_hook(0);
    return os;
  }

}
