#include "tgbabddtranslateproxy.hh"
#include "bddprint.hh"

namespace spot
{

  // tgba_bdd_translate_proxy_succ_iterator
  // --------------------------------------
  
  tgba_bdd_translate_proxy_succ_iterator::
  tgba_bdd_translate_proxy_succ_iterator(tgba_succ_iterator_concrete* it,
					 bddPair* rewrite)
    : iter_(it), rewrite_(rewrite)
  {
  }
  
  void 
  tgba_bdd_translate_proxy_succ_iterator::first()
  {
    iter_->first();
  }
  
  void 
  tgba_bdd_translate_proxy_succ_iterator::next()
  {
    iter_->next();
  }
  
  bool 
  tgba_bdd_translate_proxy_succ_iterator::done()
  {
    return iter_->done();
  }

  state_bdd* 
  tgba_bdd_translate_proxy_succ_iterator::current_state()
  {
    state_bdd* s = iter_->current_state();
    s->as_bdd() = bdd_replace(s->as_bdd(), rewrite_);
    return s;
  }

  bdd 
  tgba_bdd_translate_proxy_succ_iterator::current_condition()
  {
    return bdd_replace(iter_->current_condition(), rewrite_);
  }

  bdd 
  tgba_bdd_translate_proxy_succ_iterator::current_promise()
  {
    return bdd_replace(iter_->current_promise(), rewrite_);
  }


  // tgba_bdd_translate_proxy
  // ------------------------

  tgba_bdd_translate_proxy::tgba_bdd_translate_proxy(const tgba& from, 
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
    for (i_from = f.prom_map.begin(); i_from != f.prom_map.end(); ++i_from)
      {
	i_to = to_.prom_map.find(i_from->first);
	assert(i_to != to_.prom_map.end());
	bdd_setpair(rewrite_to_, i_from->second, i_to->second);
	bdd_setpair(rewrite_from_, i_to->second, i_from->second);
      }
  }

  tgba_bdd_translate_proxy::~tgba_bdd_translate_proxy()
  {
    bdd_freepair(rewrite_from_);
    bdd_freepair(rewrite_to_);
  }
  
  state_bdd* 
  tgba_bdd_translate_proxy::get_init_state() const
  {
    state_bdd* s = dynamic_cast<state_bdd*>(from_.get_init_state());
    assert(s);
    s->as_bdd() = bdd_replace(s->as_bdd(), rewrite_to_);
    return s;
  }

  tgba_bdd_translate_proxy_succ_iterator* 
  tgba_bdd_translate_proxy::succ_iter(const state* state) const
  {
    const state_bdd* s = dynamic_cast<const state_bdd*>(state);
    assert(s);
    state_bdd s2(bdd_replace(s->as_bdd(), rewrite_from_));    
    tgba_succ_iterator_concrete* it = 
      dynamic_cast<tgba_succ_iterator_concrete*>(from_.succ_iter(&s2));
    assert(it);
    return new tgba_bdd_translate_proxy_succ_iterator(it, rewrite_to_);
  }

  const tgba_bdd_dict& 
  tgba_bdd_translate_proxy::get_dict() const
  {
    return to_;
  }
  
  std::string
  tgba_bdd_translate_proxy::format_state(const state* state) const
  {
    const state_bdd* s = dynamic_cast<const state_bdd*>(state);
    assert(s);
    return bdd_format_set(to_, s->as_bdd());
  }


}
