#include "tgba.hh"

namespace spot
{
  tgba::tgba()
    : last_support_conditions_input_(0),
      last_support_variables_input_(0)
  {
  }

  tgba::~tgba()
  {
    if (last_support_conditions_input_)
      delete last_support_conditions_input_;
    if (last_support_variables_input_)
      delete last_support_variables_input_;
  }

  bdd
  tgba::support_conditions(const state* state) const
  {
    if (! last_support_conditions_input_
	|| last_support_conditions_input_->compare(state) != 0)
      {
	last_support_conditions_output_ =
	  compute_support_conditions(state);
	if (last_support_conditions_input_)
	  delete last_support_conditions_input_;
	last_support_conditions_input_ = state->clone();
      }
    return last_support_conditions_output_;
  }

  bdd
  tgba::support_variables(const state* state) const
  {
    if (! last_support_variables_input_
	|| last_support_variables_input_->compare(state) != 0)
      {
	last_support_variables_output_ =
	  compute_support_variables(state);
	if (last_support_variables_input_)
	  delete last_support_variables_input_;
	last_support_variables_input_ = state->clone();
      }
    return last_support_variables_output_;
  }

  state*
  tgba::project_state(const state* s, const tgba* t) const
  {
    if (t == this)
      return s->clone();
    return 0;
  }

}
