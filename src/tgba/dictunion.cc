#include <set>
#include "dictunion.hh"
#include "ltlvisit/clone.hh"
#include <bdd.h>
#include <cassert>

namespace spot
{

  tgba_bdd_dict
  tgba_bdd_dict_union(const tgba_bdd_dict& l, const tgba_bdd_dict& r)
  {
    std::set<const ltl::formula*> now;
    std::set<const ltl::formula*> var;
    std::set<const ltl::formula*> prom;

    tgba_bdd_dict::fv_map::const_iterator i;

    // Merge Now variables.
    for (i = l.now_map.begin(); i != l.now_map.end(); ++i)
      now.insert(i->first);
    for (i = r.now_map.begin(); i != r.now_map.end(); ++i)
      now.insert(i->first);

    // Merge atomic propositions.
    for (i = l.var_map.begin(); i != l.var_map.end(); ++i)
      var.insert(i->first);
    for (i = r.var_map.begin(); i != r.var_map.end(); ++i)
      var.insert(i->first);

    // Merge promises.
    for (i = l.prom_map.begin(); i != l.prom_map.end(); ++i)
      prom.insert(i->first);
    for (i = r.prom_map.begin(); i != r.prom_map.end(); ++i)
      prom.insert(i->first);

    // Ensure we have enough BDD variables.
    int have = bdd_extvarnum(0);
    int want = now.size() * 2 + var.size() + prom.size();
    if (have < want)
      bdd_setvarnum(want);

    // Fill in the "defragmented" union dictionary.

    // FIXME: Make some experiments with ordering of prom/var/now variables.
    // Maybe there is one order that usually produces smaller BDDs?

    // Next BDD variable to use.
    int v = 0;

    tgba_bdd_dict res;

    std::set<const ltl::formula*>::const_iterator f;
    for (f = prom.begin(); f != prom.end(); ++f)
      {
	clone(*f);
	res.prom_map[*f] = v;
	res.prom_formula_map[v] = *f;
	++v;
      }
    for (f = var.begin(); f != var.end(); ++f)
      {
	clone(*f);
	res.var_map[*f] = v;
	res.var_formula_map[v] = *f;
	++v;
      }
    for (f = now.begin(); f != now.end(); ++f)
      {
	clone(*f);
	res.now_map[*f] = v;
	res.now_formula_map[v] = *f;
	v += 2;
      }

    assert (v == want);
    return res;
  }

}
