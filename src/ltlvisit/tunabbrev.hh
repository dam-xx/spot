#ifndef SPOT_LTLVISIT_TUNABBREV_HH
# define SPOT_LTLVISIT_TUNABBREV_HH

#include "ltlast/formula.hh"
#include "ltlvisit/lunabbrev.hh"

namespace spot
{
  namespace ltl
  {
    /// \brief Clone and rewrite a formula to remove most of the
    /// abbreviated LTL and logical operators.
    ///
    /// The rewriting performed on logical operator is
    /// the same as the one done by spot::ltl::unabbreviate_logic_visitor.
    ///
    /// This will also rewrite unary operators such as unop::F,
    /// and unop::G, using only binop::U, and binop::R.
    ///
    /// This visitor is public, because it's convenient
    /// to derive from it and override some of its methods.
    /// But if you just want the functionality, consider using
    /// spot::ltl::unabbreviate_ltl instead.
    class unabbreviate_ltl_visitor : public unabbreviate_logic_visitor
    {
      typedef unabbreviate_logic_visitor super;
    public:
      unabbreviate_ltl_visitor();
      virtual ~unabbreviate_ltl_visitor();

      void visit(unop* uo);

      formula* recurse(formula* f);
    };

    /// \brief Clone and rewrite a formula to remove most of the
    /// abbreviated LTL and logical operators.
    ///
    /// The rewriting performed on logical operator is
    /// the same as the one done by spot::ltl::unabbreviate_logic.
    ///
    /// This will also rewrite unary operators such as unop::F,
    /// and unop::G, using only binop::U, and binop::R.
    formula* unabbreviate_ltl(const formula* f);
  }
}

#endif // SPOT_LTLVISIT_TUNABBREV_HH
