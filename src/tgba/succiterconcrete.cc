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
    // The full satisfactions of succ_set_ maybe something
    // like this (ignoring Now variables):
    //      a &  b &  Next[a] &  Next[b]
    //     !a &  b &  Next[a] &  Next[b]
    //      a & !b &  Next[a] &  Next[b]
    //      a &  b &  Next[a] & !Next[b]
    // This denotes four transitions, three of which going to
    // the same node.  Obviously (a&b | !a&b | a&!b)
    // == (a | b), so it's tempting to replace these four
    // transitions by
    //      (a + b) &  Next[a] &  Next[b]
    //       a &  b &  Next[a] & !Next[b]
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
	// FIXME: Iterating on the successors this way (calling
	// bdd_satone{,set} and NANDing out the result from a
	// set) requires several descent of the BDD.  Maybe it would be
	// faster to compute all satisfying formula in one operation.

	succ_set_left_ &= !current_;
	if (succ_set_left_ == bddfalse) // No more successors?
	  return;

	// Pick one transition, and extract its destination.
	bdd trans = bdd_satoneset(succ_set_left_, data_.next_set,
				  bddfalse);
	bdd dest = bdd_exist(trans, data_.notnext_set);

	// Gather all transitions going to this destination...
	current_ = succ_set_left_ & dest;
	// ... and compute their accepting sets.
	bdd as = data_.accepting_conditions & current_;

	// AS is false when no satisfaction of the current transition
	// belongs to an accepting set: current_ can be used as-is.
	if (as != bddfalse)
	  {
	    // Otherwise, we have accepting sets, and we should
	    // restrict current_ to a subset sharing the same
	    // accepting conditions.
	    // same accepting set.

	    as = bdd_exist(as, data_.nownext_set);
	    // as = (a | (!a)&b) & (Acc[a] | Acc[b]) + (!a & Acc[b])
	    bdd cube = bdd_satone(as);
	    // cube = (!ab & Acc[a])
	    bdd prop = bdd_exist(cube, data_.acc_set);
	    // prop = (!a)&b
	    current_acc_ = bdd_forall(bdd_restrict(as, prop), data_.var_set);
	    // current_acc_ = (Acc[a] | Acc[b])
	    assert(current_acc_ != bddfalse);
	    // Find other transitions included exactly in each of these
	    // accepting sets and are not included in other sets.
	    //   Consider
	    //      !p.!Acc[g].Acc[f] + p.!Acc[g].Acc[f] + p.Acc[g].!Acc[f]
	    //   if current_acc_ = !Acc[g].Acc[f] we
	    //   want to compute !p, not (!p + p), because p really
	    //   belongs to !Acc[g].Acc[f] + Acc[g].!Acc[f], not
	    //   only !Acc[g].Acc[f].
	    // So, first, filter out all transitions like p, which
	    // are also in other accepting sets.
	    bdd fout = bdd_relprod(as, !current_acc_, data_.acc_set);
	    bdd as_fout = as & !fout;
	    // Then, pick the remaining term that are exactly in all
	    // required accepting sets.
	    bdd all = bddtrue;
	    bdd acc = current_acc_;
	    do
	      {
		bdd one_acc = bdd_satone(acc);
		acc &= !one_acc;
		all &= bdd_relprod(as_fout, one_acc, data_.acc_set);
	      }
	    while (acc != bddfalse);
	    // all = (a | (!a)&b) & (Acc[a] | Acc[b])
	    current_ = all & dest;
	    // current_ = (a | (!a)&b) & (Next...)
	  }
	else
	  {
	    current_acc_ = bddfalse;
	  }

	assert(current_ != bddfalse);
	// The destination state, computed here, should be compatible
	// with the transition relation.  Otherwise it won't have any
	// successor (a dead node) and we can skip it.  We need to
	// compute current_state_ anyway, so this test costs us nothing.
	assert(dest == bdd_exist(current_, data_.notnext_set));
	current_state_ = bdd_replace(dest, data_.next_to_now);
      }
    while ((current_state_ & data_.relation) == bddfalse);
  }

  bool
  tgba_succ_iterator_concrete::done() const
  {
    return succ_set_left_ == bddfalse;
  }

  state_bdd*
  tgba_succ_iterator_concrete::current_state() const
  {
    assert(!done());
    return new state_bdd(current_state_);
  }

  bdd
  tgba_succ_iterator_concrete::current_condition() const
  {
    assert(!done());
    return bdd_exist(current_, data_.notvar_set);
  }

  bdd
  tgba_succ_iterator_concrete::current_accepting_conditions() const
  {
    assert(!done());
    return current_acc_;
  }

}
