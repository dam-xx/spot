#include "ltlvisit/clone.hh"
#include "ltlvisit/destroy.hh"
#include "tgbabddconcretefactory.hh"
namespace spot
{
  tgba_bdd_concrete_factory::tgba_bdd_concrete_factory()
    : now_to_next_(bdd_newpair())
  {
  }

  tgba_bdd_concrete_factory::~tgba_bdd_concrete_factory()
  {
    acc_map_::iterator ai;
    for (ai = acc_.begin(); ai != acc_.end(); ++ai)
      destroy(ai->first);
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
    bdd_setpair(now_to_next_, num, num + 1);

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
  tgba_bdd_concrete_factory::declare_accepting_condition(bdd b,
							 const ltl::formula* a)
  {
    // Maintain a conjunction of BDDs associated to A.  We will latter
    // (in tgba_bdd_concrete_factory::finish()) associate this
    // conjunction to A.
    acc_map_::iterator ai = acc_.find(a);
    if (ai == acc_.end())
      {
	a = clone(a);
	acc_[a] = b;
      }
    else
      {
	ai->second &= b;
      }
  }

  void
  tgba_bdd_concrete_factory::finish()
  {
    acc_map_::iterator ai;
    for (ai = acc_.begin(); ai != acc_.end(); ++ai)
      {
	// Register a BDD variable for this accepting condition.
	int a = create_node();
	const ltl::formula* f = clone(ai->first); // The associated formula.
	dict_.acc_map[f] = a;
	dict_.acc_formula_map[a] = f;
	bdd acc = ithvar(a);
	// Keep track of all accepting conditions for easy
	// existential quantification.
	data_.declare_accepting_condition(acc);
      }
    for (ai = acc_.begin(); ai != acc_.end(); ++ai)
      {
	bdd acc = ithvar(dict_.acc_map[ai->first]);

	// Complete acc with all the other accepting conditions negated.
	acc &= bdd_exist(data_.negacc_set, acc);

	// Any state matching the BDD formulae registered is part
	// of this accepting set.
	data_.accepting_conditions |= ai->second & acc;

	// Keep track of all accepting conditions, so that we can
	// easily check whether a transition satisfies all accepting
	// conditions.
	data_.all_accepting_conditions |= acc;
      }

    // Any constraint between Now variables also exist between Next
    // variables.  Doing this limits the quantity of useless
    // successors we will have to explore.  (By "useless successors"
    // I mean a combination of Next variables that represent a cul de sac
    // state: the combination exists but won't allow further exploration
    // because it fails the constraints.)
    data_.relation &= bdd_replace(bdd_exist(data_.relation, data_.notnow_set),
				  now_to_next_);
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
