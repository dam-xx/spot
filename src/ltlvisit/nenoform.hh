#ifndef SPOT_LTLVISIT_NENOFORM_HH
# define SPOT_LTLVISIT_NENOFORM_HH

#include "ltlast/formula.hh"
#include "ltlast/visitor.hh"

namespace spot 
{
  namespace ltl
  {
    /// \brief Build the negative normal form of \a f.
    /// 
    /// All negations of the formula are pushed in front of the 
    /// atomic propositions.
    ///
    /// \param f The formula to normalize.
    /// \param negated If \c true, return the negative normal form of
    ///        \c !f
    ///
    /// Note that this will not remove abbreviated operators.  If you
    /// want to remove abbreviations, call spot::ltl::unabbreviate_logic
    /// or spot::ltl::unabbreviate_ltl first.  (Calling these functions
    /// after spot::ltl::negative_normal_form would likely produce a
    /// formula which is not in negative normal form.)
    formula* negative_normal_form(const formula* f, bool negated = false);
  }
}

#endif //  SPOT_LTLVISIT_NENOFORM_HH
