#include "succiterconcrete.hh"
#include <cassert>

namespace spot
{
  tgba_succ_iterator_concrete::tgba_succ_iterator_concrete
  (const tgba_bdd_core_data& d, bdd successors)
    : data_(d),
      succ_set_(successors),
      succ_set_left_(successors),
      trans_dest_(bddfalse),
      trans_set_(bddfalse),
      trans_set_left_(bddfalse),
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
    trans_dest_ = bddfalse;
    trans_set_ = bddfalse;
    trans_set_left_ = bddfalse;
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
    // The full satisfactions of succ_set_ maybe something
    // like this (ignoring Now variables):
    //      a &  b &  Next[a] &  Next[b]
    //     !a &  b &  Next[a] &  Next[b]
    //      a & !b &  Next[a] &  Next[b]
    //      a &  b &  Next[a] & !Next[b]
    // This denotes four transitions, three of which going to
    // the same node.  Obviously (a&b | !a&b | a&!b)
    // == (a | b), so it's tempting to replace these three
    // transitions by the following two:
    //      a &  Next[a] &  Next[b]
    //      b &  Next[a] &  Next[b]
    // Is this always correct?  No!  It depends on the
    // accepting conditions associated to each transition.
    // We cannot merge transitions which have different
    // accepting conditions.
    // Let's label transitions with hypothetic accepting sets:
    //      a &  b &  Next[a] &  Next[b] ;  Acc[1]
    //     !a &  b &  Next[a] &  Next[b] ;  Acc[2]
    //      a & !b &  Next[a] &  Next[b] ;  Acc[2]
    //      a &  b &  Next[a] & !Next[b] ;  Acc[1]
    // Now it's pretty clear only the first two transitions
    // may be merged:
    //           b &  Next[a] &  Next[b] ;  Acc[1]
    //      a & !b &  Next[a] &  Next[b] ;  Acc[2]
    //      a &  b &  Next[a] & !Next[b] ;  Acc[1]

    do
      {
	// We performs a two-level iteration on transitions.
	//
	// succ_set_ and succ_set_left_ hold the information about the
	// outer loop: the set of all transitiong going off this
	// state.
	//
	// From this (outer) set, we compute subsets of transitions
	// going to the same state and sharing the same accepting
	// conditions.  These are held by the trans_set_ and
	// trans_set_left_ variables.
	//
	// We iterate of trans_set_ until all its transitions
	// have been seen (trans_set_left_ is then empty).  Then
	// we remove trans_set_ from succ_set_left_ and compute another
	// subset of succ_set_left_ to iterate over.

	// FIXME: Iterating on the successors this way (calling
	// bdd_satone{,set} and NANDing out the result from a
	// set) requires several descent of the BDD.  Maybe it would be
	// faster to compute all satisfying formula in one operation.

	if (trans_set_left_ == bddfalse)
	  {
	    succ_set_left_ &= !(trans_set_ & trans_dest_);
	    if (succ_set_left_ == bddfalse) // No more successors?
	      return;

	    // Pick one transition, and extract its destination.
	    bdd trans = bdd_satoneset(succ_set_left_, data_.next_set,
				      bddfalse);
	    trans_dest_ = bdd_exist(trans, data_.notnext_set);

	    // Gather all transitions going to this destination...
	    bdd st = succ_set_left_ & trans_dest_;
	    // ... and compute their accepting sets.
	    bdd as = data_.accepting_conditions & st;

	    if (as == bddfalse)
	      {
		// AS is false when no transition from ST belongs to
		// an accepting set.  Iterate over ST directly.
		trans_set_ = bdd_exist(st, data_.now_set & data_.next_set);
	      }
	    else
	      {
		// Otherwise, we have accepting sets, and we should
		// only work over a set of transitions sharing the
		// same accepting set.

		as = bdd_exist(as, data_.now_set & data_.next_set);
		// as = (a | (!a)&b) & (Acc[a] | Acc[b]) + (!a & Acc[b])
		bdd cube = bdd_satone(as);
		// cube = (!ab & Acc[a])
		bdd prop = bdd_exist(cube, data_.acc_set);
		// prop = (!a)&b
		bdd acc = bdd_forall(bdd_restrict(as, prop), data_.var_set);
		// acc = (Acc[a] | Acc[b])
		trans_set_ = bdd_restrict(as, acc);
		// trans_set = (a | (!a)&b)
	      }
	    trans_set_left_ = trans_set_;
	    neg_trans_set_ = !trans_set_;
	  }

	// Pick and remove one satisfaction from trans_set_left_.
	bdd cube = bdd_satone(trans_set_left_);
	trans_set_left_ &= !cube;
	// Let this cube grow as much as possible 
	// (e.g., cube "(!a)&b" taken from "a | (!a)&b" can
	// be simplified to "b").
	cube = bdd_simplify(cube, cube | neg_trans_set_);
	// Complete with the destination.
	current_ = cube & trans_dest_;

	// The destination state, computed here, should be compatible
	// with the transition relation.  Otherwise it won't have any
	// successor (a dead node) and we can skip it.  We need to
	// compute current_state_ anyway, so this test costs us nothing.
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
