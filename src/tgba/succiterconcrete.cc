#include "succiterconcrete.hh"
#include <cassert>

namespace spot
{
  tgba_succ_iterator_concrete::tgba_succ_iterator_concrete
  (const tgba_bdd_core_data& d, bdd successors)
    : data_(d),
      succ_set_(successors),
      succ_set_left_(successors),
      current_(bddfalse),
      current_base_(bddfalse),
      current_base_left_(bddfalse)
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
    // Calling bdd_fullsatone on succ_set_ would compute many more
    // transitions (twice as much on this example), since all
    // atomic propositions would have to appear.
    //
    // Basically, we want to use bdd_satone for atomic propositions
    // and bdd_fullsatone for Next variable.  We achieve this as
    // follows
    //   (1) compute a (non-full) satisfaction of succ_set_
    //       e.g.  Now[a] * !Now[b] * c * Next[a]
    //   (2) retain only the atomic propositions (here: c), and call it
    //       our current base
    //   (3) compute a full satisfaction for succ_set_ & current_base_
    //        Now[a] * !Now[b] * c * d * Next[a] * Next[b]
    //   (4) retain only the Next variables
    //       Next[a] * Next[b]
    //   (5) and finally append them to the current base
    //       c * Next[a] * Next[b]
    //       This is the first successor.
    //
    // This algorithm would compute only one successor.  In order to
    // iterate over the other ones, we have a couple of variables that
    // remember what has been explored so far and what is yet to
    // explore.
    //
    // The next time we compute a successor, we start at (3) and
    // compute the *next* full satisfaction matching current_base_.  The
    // current_base_left_ serves this purpose: holding all the
    // current_base_ satisfactions that haven't yet been explored.
    //
    // Once we've explored all satisfactions of current_base_, we
    // start over at (1) and compute the *next* (non-full)
    // satisfaction of succ_set_.  To that effect, the variable
    // next_succ_set hold all these satisfactions that must still be
    // explored.
    //
    // Note: on this example we would not exactly get the six transitions
    // mentionned, but maybe something like
    //     c * Next[a] * Next[b]
    //     c * Next[a] * !Next[b]
    //     c * !Next[a] * Next[b]
    //     d * !c * Next[a] * Next[b]
    //     d * !c * Next[a] * !Next[b]
    //     d * !c * !Next[a] * Next[b]
    // depending on the BDD order.

    // FIXME: It would help to have this half-full half-partial
    // satisfaction operation available as one BDD function.
    // FIXME: Iterating on the successors this way (calling
    // bdd_satone/bdd_fullsetone and NANDing out the result from a
    // set) requires several descent of the BDD.  Maybe it would be
    // faster to compute all satisfying formula in one operation.
    do
      {
	if (current_ == bddfalse)
	  {
	    succ_set_left_ &= !current_base_;
	    if (succ_set_left_ == bddfalse) // No more successors?
	      return;
	    // (1) (2)
	    current_base_ = bdd_exist(bdd_satone(succ_set_left_),
				      data_.notvar_set);
	    current_base_left_ = current_base_;
	  }
	// (3) (4) (5)
	current_ =
	  current_base_ & bdd_exist(bdd_fullsatone(succ_set_left_
						   & current_base_left_),
				    data_.notnext_set);
	current_base_left_ &= !current_;
      }
    while (// No more successors with the current_base_? Compute
	   // the next base.
	   current_ == bddfalse
	   // The destination state, computed here, should be
	   // compatible with the transition relation.  Otherwise
	   // it won't have any successor and useless.
	   || ((current_state_ = bdd_replace(bdd_exist(current_,
						       data_.notnext_set),
					     data_.next_to_now))
	       & data_.relation) == bddfalse);
  }

  bool
  tgba_succ_iterator_concrete::done()
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
