#include "tgbabddprod.hh"
#include "tgbabddtranslateproxy.hh"
#include "dictunion.hh"

namespace spot
{

  ////////////////////////////////////////////////////////////
  // state_bdd_product

  state_bdd_product::~state_bdd_product()
  {
    delete left_;
    delete right_;
  }

  int 
  state_bdd_product::compare(const state* other) const
  {
    const state_bdd_product* o = dynamic_cast<const state_bdd_product*>(other);
    assert(o);
    int res = left_->compare(o->left());
    if (res != 0)
      return res;
    return right_->compare(o->right());
  }

  ////////////////////////////////////////////////////////////
  // tgba_bdd_product_succ_iterator

  tgba_bdd_product_succ_iterator::tgba_bdd_product_succ_iterator
  (tgba_succ_iterator* left, tgba_succ_iterator* right)
    : left_(left), right_(right)
  {
  }
  
  void
  tgba_bdd_product_succ_iterator::step_()
  {
    left_->next();
    if (left_->done())
      {
	left_->first();
	right_->next();
      }
  }

  void
  tgba_bdd_product_succ_iterator::next_non_false_()
  {
    while (!done())
      {
	bdd l = left_->current_condition();
	bdd r = right_->current_condition();
	bdd current_cond = l & r;

	if (current_cond != bddfalse)
	  {
	    current_cond_ = current_cond;
	    return;
	  }
	step_();
      }
  }

  void 
  tgba_bdd_product_succ_iterator::first()
  {
    left_->first();
    right_->first();
    next_non_false_();
  }

  void 
  tgba_bdd_product_succ_iterator::next()
  {
    step_();
    next_non_false_();
  }

  bool 
  tgba_bdd_product_succ_iterator::done()
  {
    return right_->done();
  }
  
  
  state_bdd* 
  tgba_bdd_product_succ_iterator::current_state()
  {
    state_bdd* ls = dynamic_cast<state_bdd*>(left_->current_state());
    state_bdd* rs = dynamic_cast<state_bdd*>(right_->current_state());
    assert(ls);
    assert(rs);
    return new state_bdd_product(ls, rs);
  }
  
  bdd 
  tgba_bdd_product_succ_iterator::current_condition()
  {
    return current_cond_;
  }
   
  bdd tgba_bdd_product_succ_iterator::current_promise()
  {
    return left_->current_promise() & right_->current_promise();
  }

  ////////////////////////////////////////////////////////////
  // tgba_bdd_product

  tgba_bdd_product::tgba_bdd_product(const tgba& left, const tgba& right)
    : dict_(tgba_bdd_dict_union(left.get_dict(), right.get_dict()))
  {
    // Translate the left automaton if needed.
    if (dict_.contains(left.get_dict()))
      {
	left_ = &left;
	left_should_be_freed_ = false;
      }
    else
      {
	left_ = new tgba_bdd_translate_proxy(left, dict_);
	left_should_be_freed_ = true;
      }

    // Translate the right automaton if needed.
    if (dict_.contains(right.get_dict()))
      {
	right_ = &right;
	right_should_be_freed_ = false;
      }
    else
      {
	right_ = new tgba_bdd_translate_proxy(right, dict_);
	right_should_be_freed_ = true;
      }
  }

  tgba_bdd_product::~tgba_bdd_product()
  {
    if (left_should_be_freed_)
      delete left_;
    if (right_should_be_freed_)
      delete right_;
  }

  state* 
  tgba_bdd_product::get_init_state() const
  {
    state_bdd* ls = dynamic_cast<state_bdd*>(left_->get_init_state());
    state_bdd* rs = dynamic_cast<state_bdd*>(right_->get_init_state());
    assert(ls);
    assert(rs);
    return new state_bdd_product(ls, rs);    
  }

  tgba_bdd_product_succ_iterator* 
  tgba_bdd_product::succ_iter(const state* state) const
  {
    const state_bdd_product* s = dynamic_cast<const state_bdd_product*>(state);
    assert(s);

    tgba_succ_iterator* li = left_->succ_iter(s->left());
    tgba_succ_iterator* ri = right_->succ_iter(s->right());
    return new tgba_bdd_product_succ_iterator(li, ri);
  }

  const tgba_bdd_dict& 
  tgba_bdd_product::get_dict() const
  {
    return dict_;
  }

  std::string 
  tgba_bdd_product::format_state(const state* state) const
  {
    const state_bdd_product* s = dynamic_cast<const state_bdd_product*>(state);
    assert(s);
    return (left_->format_state(s->left()) 
	    + " * " 
	    + right_->format_state(s->right()));
  }
  
  
}
