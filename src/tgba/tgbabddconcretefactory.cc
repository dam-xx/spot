#include "ltlvisit/clone.hh"
#include "ltlvisit/destroy.hh"
#include "tgbabddconcretefactory.hh"

namespace spot
{
  tgba_bdd_concrete_factory::~tgba_bdd_concrete_factory()
  {
    promise_map_::iterator pi;
    for (pi = prom_.begin(); pi != prom_.end(); ++pi)
      destroy(pi->first);
  }

  int
  tgba_bdd_concrete_factory::create_state(const ltl::formula* f)
  {
    // Do not build a state that already exists.
    tgba_bdd_dict::fv_map::iterator sii = dict_.now_map.find(f);
    if (sii != dict_.now_map.end())
      return sii->second;

    f = clone(f);

    int num = create_pair();
    dict_.now_map[f] = num;
    dict_.now_formula_map[num] = f;

    // Record that num+1 should be renamed as num when
    // the next state becomes current.
    bdd_setpair(data_.next_to_now, num + 1, num);

    // Keep track of all "Now" variables for easy
    // existential quantification.
    data_.declare_now_next (ithvar(num), ithvar(num + 1));
    return num;
  }

  int
  tgba_bdd_concrete_factory::create_atomic_prop(const ltl::formula* f)
  {
    // Do not build a variable that already exists.
    tgba_bdd_dict::fv_map::iterator sii = dict_.var_map.find(f);
    if (sii != dict_.var_map.end())
      return sii->second;

    f = clone(f);

    int num = create_node();
    dict_.var_map[f] = num;
    dict_.var_formula_map[num] = f;

    // Keep track of all atomic proposition for easy
    // existential quantification.
    data_.declare_atomic_prop(ithvar(num));
    return num;
  }

  void
  tgba_bdd_concrete_factory::declare_promise(bdd b,
					     const ltl::formula* p)
  {
    // Maintain a disjunction of BDDs associated to P.
    // We will latter (in tgba_bdd_concrete_factory::finish())
    // record this disjunction as equivalant to P.
    promise_map_::iterator pi = prom_.find(p);
    if (pi == prom_.end())
      {
	p = clone(p);
	prom_[p] = b;
      }
    else
      {
	pi->second |= b;
      }
  }

  void
  tgba_bdd_concrete_factory::finish()
  {
    promise_map_::iterator pi;
    for (pi = prom_.begin(); pi != prom_.end(); ++pi)
      {
	// Register a BDD variable for this promise.
	int p = create_node();
	const ltl::formula* f = clone(pi->first); // The promised formula.
	dict_.prom_map[f] = p;
	dict_.prom_formula_map[p] = f;

	bdd prom = ithvar(p);
	// Keep track of all promises for easy existential quantification.
	data_.declare_promise(prom);

	// The promise P must hold if we have to verify any of the
	// (BDD) formulae registered.
	add_relation(bdd_apply(prom, pi->second, bddop_biimp));
      }
  }

  const tgba_bdd_core_data&
  tgba_bdd_concrete_factory::get_core_data() const
  {
    return data_;
  }

  const tgba_bdd_dict&
  tgba_bdd_concrete_factory::get_dict() const
  {
    return dict_;
  }

  void
  tgba_bdd_concrete_factory::add_relation(bdd new_rel)
  {
    data_.relation &= new_rel;
  }
}
