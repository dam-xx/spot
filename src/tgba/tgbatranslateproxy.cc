#include "tgbatranslateproxy.hh"
#include "bddprint.hh"
#include <cassert>

namespace spot
{

  // tgba_translate_proxy_succ_iterator
  // --------------------------------------

  tgba_translate_proxy_succ_iterator::
  tgba_translate_proxy_succ_iterator(tgba_succ_iterator* it,
				     bddPair* rewrite)
    : iter_(it), rewrite_(rewrite)
  {
  }

  tgba_translate_proxy_succ_iterator::~tgba_translate_proxy_succ_iterator()
  {
    delete iter_;
  }

  void
  tgba_translate_proxy_succ_iterator::first()
  {
    iter_->first();
  }

  void
  tgba_translate_proxy_succ_iterator::next()
  {
    iter_->next();
  }

  bool
  tgba_translate_proxy_succ_iterator::done() const
  {
    return iter_->done();
  }

  state*
  tgba_translate_proxy_succ_iterator::current_state()
  {
    state* s = iter_->current_state();
    s->translate(rewrite_);
    return s;
  }

  bdd
  tgba_translate_proxy_succ_iterator::current_condition()
  {
    return bdd_replace(iter_->current_condition(), rewrite_);
  }

  bdd
  tgba_translate_proxy_succ_iterator::current_accepting_conditions()
  {
    return bdd_replace(iter_->current_accepting_conditions(), rewrite_);
  }


  // tgba_translate_proxy
  // ------------------------

  tgba_translate_proxy::tgba_translate_proxy(const tgba& from,
					     const tgba_bdd_dict& to)
    : from_(from), to_(to)
  {
    const tgba_bdd_dict& f = from.get_dict();
    rewrite_to_ = bdd_newpair();
    rewrite_from_ = bdd_newpair();

    tgba_bdd_dict::fv_map::const_iterator i_from;
    tgba_bdd_dict::fv_map::const_iterator i_to;

    for (i_from = f.now_map.begin(); i_from != f.now_map.end(); ++i_from)
      {
	i_to = to_.now_map.find(i_from->first);
	assert(i_to != to_.now_map.end());
	bdd_setpair(rewrite_to_, i_from->second, i_to->second);
	bdd_setpair(rewrite_to_, i_from->second + 1, i_to->second + 1);
	bdd_setpair(rewrite_from_, i_to->second, i_from->second);
	bdd_setpair(rewrite_from_, i_to->second + 1, i_from->second + 1);
      }
    for (i_from = f.var_map.begin(); i_from != f.var_map.end(); ++i_from)
      {
	i_to = to_.var_map.find(i_from->first);
	assert(i_to != to_.var_map.end());
	bdd_setpair(rewrite_to_, i_from->second, i_to->second);
	bdd_setpair(rewrite_from_, i_to->second, i_from->second);
      }
    for (i_from = f.acc_map.begin(); i_from != f.acc_map.end(); ++i_from)
      {
	i_to = to_.acc_map.find(i_from->first);
	assert(i_to != to_.acc_map.end());
	bdd_setpair(rewrite_to_, i_from->second, i_to->second);
	bdd_setpair(rewrite_from_, i_to->second, i_from->second);
      }

    all_accepting_conditions_ = bdd_replace(from.all_accepting_conditions(),
					    rewrite_to_);
    neg_accepting_conditions_ = bdd_replace(from.neg_accepting_conditions(),
					    rewrite_to_);
  }

  tgba_translate_proxy::~tgba_translate_proxy()
  {
    bdd_freepair(rewrite_from_);
    bdd_freepair(rewrite_to_);
  }

  state*
  tgba_translate_proxy::get_init_state() const
  {
    state* s = from_.get_init_state();
    s->translate(rewrite_to_);
    return s;
  }

  tgba_translate_proxy_succ_iterator*
  tgba_translate_proxy::succ_iter(const state* s) const
  {
    state* s2 = s->clone();
    s2->translate(rewrite_from_);
    tgba_translate_proxy_succ_iterator *res =
      new tgba_translate_proxy_succ_iterator(from_.succ_iter(s2),
						 rewrite_to_);
    delete s2;
    return res;
  }

  const tgba_bdd_dict&
  tgba_translate_proxy::get_dict() const
  {
    return to_;
  }

  std::string
  tgba_translate_proxy::format_state(const state* s) const
  {
    state* s2 = s->clone();
    s2->translate(rewrite_from_);
    std::string res = from_.format_state(s2);
    delete s2;
    return res;
  }

  bdd
  tgba_translate_proxy::all_accepting_conditions() const
  {
    return all_accepting_conditions_;
  }

  bdd tgba_translate_proxy::neg_accepting_conditions() const
  {
    return neg_accepting_conditions_;
  }

}
