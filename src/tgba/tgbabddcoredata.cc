#include "tgbabddcoredata.hh"

namespace spot
{
  tgba_bdd_core_data::tgba_bdd_core_data()
    : relation(bddtrue),
      now_set(bddtrue), negnow_set(bddtrue), notnow_set(bddtrue),
      notvar_set(bddtrue), notprom_set(bddtrue), next_to_now(bdd_newpair())
  {
  }

  tgba_bdd_core_data::tgba_bdd_core_data(const tgba_bdd_core_data& copy)
    : relation(copy.relation),
      now_set(copy.now_set), negnow_set(copy.negnow_set),
      notnow_set(copy.notnow_set), notvar_set(copy.notvar_set),
      notprom_set(copy.notprom_set),
      next_to_now(bdd_copypair(copy.next_to_now))
  {
  }

  // Merge two core_data.
  tgba_bdd_core_data::tgba_bdd_core_data(const tgba_bdd_core_data& left,
					   const tgba_bdd_core_data& right)
    : relation(left.relation & right.relation),
      now_set(left.now_set & right.now_set),
      negnow_set(left.negnow_set & right.negnow_set),
      notnow_set(left.notnow_set & right.notnow_set),
      notvar_set(left.notvar_set & right.notvar_set),
      notprom_set(left.notprom_set & right.notprom_set),
      next_to_now(bdd_mergepairs(left.next_to_now, right.next_to_now))
  {
  }

  const tgba_bdd_core_data&
  tgba_bdd_core_data::operator= (const tgba_bdd_core_data& copy)
  {
    if (this != &copy)
      {
	this->~tgba_bdd_core_data();
	new (this) tgba_bdd_core_data(copy);
      }
    return *this;
  }

  tgba_bdd_core_data::~tgba_bdd_core_data()
  {
    bdd_freepair(next_to_now);
  }

  void
  tgba_bdd_core_data::declare_now_next(bdd now, bdd next)
  {
    now_set &= now;
    negnow_set &= !now;
    notnow_set &= next;
    bdd both = now & next;
    notvar_set &= both;
    notprom_set &= both;
  }

  void
  tgba_bdd_core_data::declare_atomic_prop(bdd var)
  {
    notnow_set &= var;
    notprom_set &= var;
  }

  void
  tgba_bdd_core_data::declare_promise(bdd prom)
  {
    notnow_set &= prom;
    notvar_set &= prom;
  }
}
