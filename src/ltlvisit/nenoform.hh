#ifndef SPOT_LTLVISIT_NENOFORM_HH
# define SPOT_LTLVISIT_NENOFORM_HH

#include "ltlast/formula.hh"
#include "ltlast/visitor.hh"

namespace spot 
{
  namespace ltl
  {
    /* Return the negative normal form of F, i.e., all negations
       of the formula are pushed in front of the atomic propositions.
       If NEGATED is true, return the normal form of !F instead.  */
    formula* negative_normal_form(const formula* f, bool negated = false);
  }
}

#endif //  SPOT_LTLVISIT_NENOFORM_HH
