#include "tgbabddcoredata.hh"

namespace spot
{
  tgba_bdd_core_data::tgba_bdd_core_data()
    : relation(bddtrue),
      accepting_conditions(bddfalse),
      now_set(bddtrue),
      next_set(bddtrue),
      negnow_set(bddtrue),
      notnow_set(bddtrue),
      notnext_set(bddtrue),
      var_set(bddtrue),
      notvar_set(bddtrue),
      varandnext_set(bddtrue),
      notacc_set(bddtrue),
      negacc_set(bddtrue),
      next_to_now(bdd_newpair())
  {
  }

  tgba_bdd_core_data::tgba_bdd_core_data(const tgba_bdd_core_data& copy)
    : relation(copy.relation),
      accepting_conditions(copy.accepting_conditions),
      now_set(copy.now_set),
      next_set(copy.next_set),
      negnow_set(copy.negnow_set),
      notnow_set(copy.notnow_set),
      notnext_set(copy.notnext_set),
      var_set(copy.var_set),
      notvar_set(copy.notvar_set),
      varandnext_set(copy.varandnext_set),
      notacc_set(copy.notacc_set),
      negacc_set(copy.negacc_set),
      next_to_now(bdd_copypair(copy.next_to_now))
  {
  }

  // Merge two core_data.
  tgba_bdd_core_data::tgba_bdd_core_data(const tgba_bdd_core_data& left,
					   const tgba_bdd_core_data& right)
    : relation(left.relation & right.relation),
      accepting_conditions(left.accepting_conditions
			   | right.accepting_conditions),
      now_set(left.now_set & right.now_set),
      next_set(left.next_set & right.next_set),
      negnow_set(left.negnow_set & right.negnow_set),
      notnow_set(left.notnow_set & right.notnow_set),
      notnext_set(left.notnext_set & right.notnext_set),
      var_set(left.var_set & right.var_set),
      notvar_set(left.notvar_set & right.notvar_set),
      varandnext_set(left.varandnext_set & right.varandnext_set),
      notacc_set(left.notacc_set & right.notacc_set),
      negacc_set(left.negacc_set & right.negacc_set),
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
    next_set &= next;
    negnow_set &= !now;
    notnext_set &= now;
    notnow_set &= next;
    bdd both = now & next;
    notvar_set &= both;
    notacc_set &= both;
    varandnext_set &= next;
  }

  void
  tgba_bdd_core_data::declare_atomic_prop(bdd var)
  {
    notnow_set &= var;
    notnext_set &= var;
    notacc_set &= var;
    var_set &= var;
    varandnext_set &= var;
  }

  void
  tgba_bdd_core_data::declare_accepting_condition(bdd acc)
  {
    notnow_set &= acc;
    notnext_set &= acc;
    notvar_set &= acc;
    negacc_set &= !acc;
  }
}
