#include "succiterconcrete.hh"
#include <cassert>

namespace spot
{
  tgba_succ_iterator_concrete::tgba_succ_iterator_concrete
  (const tgba_bdd_core_data& d, bdd successors)
    : data_(d),
      succ_set_(successors),
      succ_set_left_(successors),
      current_(bddfalse)
  {
  }

  tgba_succ_iterator_concrete::~tgba_succ_iterator_concrete()
  {
  }

  void
  tgba_succ_iterator_concrete::first()
  {
    succ_set_left_ = succ_set_;
    current_ = bddfalse;
    if (!done())
      next();
  }

  void
  tgba_succ_iterator_concrete::next()
  {
    assert(!done());
    // succ_set_ is the set of successors we have to explore.  it
    // contains Now/Next variable and atomic propositions.  Each
    // satisfaction of succ_set_ represents a transition, and we want
    // to compute as little transitions as possible.  However one
    // important constraint is that all Next variables must appear in
    // the satisfaction.
    //
    // For instance if succ_set_ was
    //     Now[a] * !Now[b] * (c + d) * (Next[a] + Next[b])
    // we'd like to enumerate the following six transitions
    //     c * Next[a] * Next[b]
    //     c * Next[a] * !Next[b]
    //     c * !Next[a] * Next[b]
    //     d * Next[a] * Next[b]
    //     d * Next[a] * !Next[b]
    //     d * !Next[a] * Next[b]
    // (We don't really care about the Now variables here.)
    //
    // Note: on this example it's ok to get something like
    //     c * Next[a] * Next[b]
    //     c * Next[a] * !Next[b]
    //     c * !Next[a] * Next[b]
    //     d * !c * Next[a] * Next[b]
    //     d * !c * Next[a] * !Next[b]
    //     d * !c * !Next[a] * Next[b]
    // depending on the BDD order.  It doesn't really matter.  The important
    // point is that we don't want to list all four possible 'c' and 'd' 
    // combinations.
    // FIXME: This is not what we do now: we list all possible combinations
    // of atomic propositions.

    // FIXME: Iterating on the successors this way (calling
    // bdd_satone/bdd_fullsetone and NANDing out the result from a
    // set) requires several descent of the BDD.  Maybe it would be
    // faster to compute all satisfying formula in one operation.
    do
      {
	succ_set_left_ &= !current_;
	if (succ_set_left_ == bddfalse) // No more successors?
	  return;
	current_ = bdd_satoneset(succ_set_left_,
				 data_.varandnext_set, bddfalse);
	// The destination state, computed here, should be
	// compatible with the transition relation.  Otherwise
	// it won't have any successor (a dead node).
	current_state_ = bdd_replace(bdd_exist(current_, data_.notnext_set),
				     data_.next_to_now);
      }
    while ((current_state_ & data_.relation) == bddfalse);
  }

  bool
  tgba_succ_iterator_concrete::done() const
  {
    return succ_set_left_ == bddfalse;
  }

  state_bdd*
  tgba_succ_iterator_concrete::current_state()
  {
    assert(!done());
    return new state_bdd(current_state_);
  }

  bdd
  tgba_succ_iterator_concrete::current_condition()
  {
    assert(!done());
    return bdd_exist(current_, data_.notvar_set);
  }

  bdd
  tgba_succ_iterator_concrete::current_accepting_conditions()
  {
    assert(!done());
    return bdd_exist(bdd_forall(bdd_restrict(data_.accepting_conditions,
					     current_),
				data_.var_set),
		     data_.notacc_set);
  }

}
