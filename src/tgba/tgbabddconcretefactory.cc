#include "tgbabddconcretefactory.hh"

namespace spot 
{
  tgba_bdd_concrete_factory::~tgba_bdd_concrete_factory()
  {
  }

  int
  tgba_bdd_concrete_factory::create_state(const ltl::formula* f)
  {
    // Do not build a state that already exists.
    tgba_bdd_dict::fv_map::iterator sii = dict_.now_map.find(f);
    if (sii != dict_.now_map.end())
      return sii->second;

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

    int num = create_node();
    dict_.var_map[f] = num;
    dict_.var_formula_map[num] = f;

    // Keep track of all atomic proposition for easy
    // existential quantification.
    data_.declare_atomic_prop(ithvar(num));
    return num;
  }

  int
  tgba_bdd_concrete_factory::create_promise(const ltl::formula* f)
  {
    // Do not build a promise that already exists.
    tgba_bdd_dict::fv_map::iterator sii = dict_.prom_map.find(f);
    if (sii != dict_.prom_map.end())
      return sii->second;

    int num = create_node();
    dict_.prom_map[f] = num;
    dict_.prom_formula_map[num] = f;

    // Keep track of all promises for easy existential quantification.
    data_.declare_promise(ithvar(num));
    return num;
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
