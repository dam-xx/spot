#include "tgbaproduct.hh"
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

  tgba_product::tgba_product(const tgba* left, const tgba* right)
    : dict_(left->get_dict()), left_(left), right_(right)
  {
    assert(dict_ == right->get_dict());

    all_accepting_conditions_ = ((left_->all_accepting_conditions()
				  & right_->neg_accepting_conditions())
				 | (right_->all_accepting_conditions()
				    & left_->neg_accepting_conditions()));
    neg_accepting_conditions_ = (left_->neg_accepting_conditions()
				 & right_->neg_accepting_conditions());
    dict_->register_all_variables_of(&left_, this);
    dict_->register_all_variables_of(&right_, this);
  }

  tgba_product::~tgba_product()
  {
    dict_->unregister_all_my_variables(this);
  }

  state*
  tgba_product::get_init_state() const
  {
    return new state_bdd_product(left_->get_init_state(),
				 right_->get_init_state());
  }

  tgba_product_succ_iterator*
  tgba_product::succ_iter(const state* local_state,
			  const state* global_state,
			  const tgba* global_automaton) const
  {
    const state_bdd_product* s =
      dynamic_cast<const state_bdd_product*>(local_state);
    assert(s);

    // If global_automaton is not specified, THIS is the root of a
    // product tree.
    if (! global_automaton)
      {
	global_automaton = this;
	global_state = local_state;
      }

    tgba_succ_iterator* li = left_->succ_iter(s->left(),
					      global_state, global_automaton);
    tgba_succ_iterator* ri = right_->succ_iter(s->right(),
					       global_state, global_automaton);
    return new tgba_product_succ_iterator(li, ri,
					  left_->neg_accepting_conditions(),
					  right_->neg_accepting_conditions());
  }

  bdd
  tgba_product::compute_support_conditions(const state* in) const
  {
    const state_bdd_product* s =
      dynamic_cast<const state_bdd_product*>(in);
    assert(s);
    bdd lsc = left_->support_conditions(s->left());
    bdd rsc = right_->support_conditions(s->right());
    return lsc & rsc;
  }

  bdd
  tgba_product::compute_support_variables(const state* in) const
  {
    const state_bdd_product* s =
      dynamic_cast<const state_bdd_product*>(in);
    assert(s);
    bdd lsc = left_->support_variables(s->left());
    bdd rsc = right_->support_variables(s->right());
    return lsc & rsc;
  }

  bdd_dict*
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

  state*
  tgba_product::project_state(const state* s, const tgba* t) const
  {
    const state_bdd_product* s2 = dynamic_cast<const state_bdd_product*>(s);
    assert(s2);
    if (t == this)
      return s2->clone();
    state* res = left_->project_state(s2->left(), t);
    if (res)
      return res;
    return right_->project_state(s2->right(), t);
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
