#include "tgbaproduct.hh"
#include "tgbatranslateproxy.hh"
#include "dictunion.hh"
#include <string>
#include <cassert>

namespace spot
{

  ////////////////////////////////////////////////////////////
  // state_bdd_product

  state_bdd_product::state_bdd_product(const state_bdd_product& o)
    : state(),
      left_(o.left()->clone()),
      right_(o.right()->clone())
  {
  }

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

  state_bdd_product*
  state_bdd_product::clone() const
  {
    return new state_bdd_product(*this);
  }

  ////////////////////////////////////////////////////////////
  // tgba_product_succ_iterator

  tgba_product_succ_iterator::tgba_product_succ_iterator
  (tgba_succ_iterator* left, tgba_succ_iterator* right,
   bdd left_neg, bdd right_neg)
    : left_(left), right_(right),
      left_neg_(left_neg),
      right_neg_(right_neg)
  {
  }

  tgba_product_succ_iterator::~tgba_product_succ_iterator()
  {
    delete left_;
    if (right_)
      delete right_;
  }

  void
  tgba_product_succ_iterator::step_()
  {
    left_->next();
    if (left_->done())
      {
	left_->first();
	right_->next();
      }
  }

  void
  tgba_product_succ_iterator::next_non_false_()
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
  tgba_product_succ_iterator::first()
  {
    if (!right_)
      return;

    left_->first();
    right_->first();
    // If one of the two successor set is empty initially, we reset
    // right_, so that done() can detect this situation easily.  (We
    // choose to reset right_ because this variable is already used by
    // done().)
    if (left_->done() || right_->done())
      {
	delete right_;
	right_ = 0;
	return;
      }
    next_non_false_();
  }

  void
  tgba_product_succ_iterator::next()
  {
    step_();
    next_non_false_();
  }

  bool
  tgba_product_succ_iterator::done() const
  {
    return !right_ || right_->done();
  }


  state_bdd_product*
  tgba_product_succ_iterator::current_state() const
  {
    return new state_bdd_product(left_->current_state(),
				 right_->current_state());
  }

  bdd
  tgba_product_succ_iterator::current_condition() const
  {
    return current_cond_;
  }

  bdd tgba_product_succ_iterator::current_accepting_conditions() const
  {
    return ((left_->current_accepting_conditions() & right_neg_)
	    | (right_->current_accepting_conditions() & left_neg_));
  }

  ////////////////////////////////////////////////////////////
  // tgba_product

  tgba_product::tgba_product(const tgba& left, const tgba& right)
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
	left_ = new tgba_translate_proxy(left, dict_);
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
	right_ = new tgba_translate_proxy(right, dict_);
	right_should_be_freed_ = true;
      }

    all_accepting_conditions_ = ((left_->all_accepting_conditions()
				  & right_->neg_accepting_conditions())
				 | (right_->all_accepting_conditions()
				    & left_->neg_accepting_conditions()));
    neg_accepting_conditions_ = (left_->neg_accepting_conditions()
				 & right_->neg_accepting_conditions());
  }

  tgba_product::~tgba_product()
  {
    if (left_should_be_freed_)
      delete left_;
    if (right_should_be_freed_)
      delete right_;
  }

  state*
  tgba_product::get_init_state() const
  {
    return new state_bdd_product(left_->get_init_state(),
				 right_->get_init_state());
  }

  tgba_product_succ_iterator*
  tgba_product::succ_iter(const state* state) const
  {
    const state_bdd_product* s = dynamic_cast<const state_bdd_product*>(state);
    assert(s);

    tgba_succ_iterator* li = left_->succ_iter(s->left());
    tgba_succ_iterator* ri = right_->succ_iter(s->right());
    return new tgba_product_succ_iterator(li, ri,
					  left_->neg_accepting_conditions(),
					  right_->neg_accepting_conditions());
  }

  const tgba_bdd_dict&
  tgba_product::get_dict() const
  {
    return dict_;
  }

  std::string
  tgba_product::format_state(const state* state) const
  {
    const state_bdd_product* s = dynamic_cast<const state_bdd_product*>(state);
    assert(s);
    return (left_->format_state(s->left())
	    + " * "
	    + right_->format_state(s->right()));
  }

  bdd
  tgba_product::all_accepting_conditions() const
  {
    return all_accepting_conditions_;
  }

  bdd
  tgba_product::neg_accepting_conditions() const
  {
    return neg_accepting_conditions_;
  }

}
