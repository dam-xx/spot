#include "tgbabddtranslatefactory.hh"
#include "dictunion.hh"
#include <cassert>

namespace spot
{
  tgba_bdd_translate_factory::tgba_bdd_translate_factory
  (const tgba_bdd_concrete& from, const tgba_bdd_dict& to)
    : dict_(to)
  {
    bddPair* rewrite = compute_pairs(from.get_dict());

    const tgba_bdd_core_data& in = from.get_core_data();

    data_.relation = bdd_replace(in.relation, rewrite);
    data_.accepting_conditions = bdd_replace(in.accepting_conditions, rewrite);
    data_.now_set = bdd_replace(in.now_set, rewrite);
    data_.next_set = bdd_replace(in.next_set, rewrite);
    data_.negnow_set = bdd_replace(in.negnow_set, rewrite);
    data_.notnow_set = bdd_replace(in.notnow_set, rewrite);
    data_.notnext_set = bdd_replace(in.notnext_set, rewrite);
    data_.notvar_set = bdd_replace(in.notvar_set, rewrite);
    data_.var_set = bdd_replace(in.var_set, rewrite);
    data_.varandnext_set = bdd_replace(in.varandnext_set, rewrite);
    data_.acc_set = bdd_replace(in.acc_set, rewrite);
    data_.notacc_set = bdd_replace(in.notacc_set, rewrite);
    data_.negacc_set = bdd_replace(in.negacc_set, rewrite);

    init_ = bdd_replace(from.get_init_bdd(), rewrite);

    bdd_freepair(rewrite);
  }

  tgba_bdd_translate_factory::~tgba_bdd_translate_factory()
  {
  }

  bddPair*
  tgba_bdd_translate_factory::compute_pairs(const tgba_bdd_dict& from)
  {
    bddPair* rewrite = bdd_newpair();

    tgba_bdd_dict::fv_map::const_iterator i_from;
    tgba_bdd_dict::fv_map::const_iterator i_to;

    for (i_from = from.now_map.begin(); i_from != from.now_map.end(); ++i_from)
      {
	i_to = dict_.now_map.find(i_from->first);
	assert(i_to != dict_.now_map.end());
	bdd_setpair(rewrite, i_from->second, i_to->second);
	bdd_setpair(rewrite, i_from->second + 1, i_to->second + 1);
	bdd_setpair(data_.next_to_now, i_to->second + 1, i_to->second);
      }
    for (i_from = from.var_map.begin(); i_from != from.var_map.end(); ++i_from)
      {
	i_to = dict_.var_map.find(i_from->first);
	assert(i_to != dict_.var_map.end());
	bdd_setpair(rewrite, i_from->second, i_to->second);
      }
    for (i_from = from.acc_map.begin();
	 i_from != from.acc_map.end();
	 ++i_from)
      {
	i_to = dict_.acc_map.find(i_from->first);
	assert(i_to != dict_.acc_map.end());
	bdd_setpair(rewrite, i_from->second, i_to->second);
      }
    return rewrite;
  }

  const tgba_bdd_core_data&
  tgba_bdd_translate_factory::get_core_data() const
  {
    return data_;
  }

  const tgba_bdd_dict&
  tgba_bdd_translate_factory::get_dict() const
  {
    return dict_;
  }

  bdd
  tgba_bdd_translate_factory::get_init_state() const
  {
    return init_;
  }


  tgba_bdd_concrete
  defrag(const tgba_bdd_concrete& a)
  {
    const tgba_bdd_dict& ad = a.get_dict();
    tgba_bdd_dict u = tgba_bdd_dict_union(ad, ad);
    tgba_bdd_translate_factory f(a, u);
    return tgba_bdd_concrete(f, f.get_init_state());
  }
}
