#ifndef SPOT_LTLVISIT_LUNABBREV_HH
# define SPOT_LTLVISIT_LUNABBREV_HH

#include "clone.hh"

namespace spot
{
  namespace ltl
  {
    /// \brief Clone and rewrite a formula to remove most of the
    /// abbreviated logical operators.
    ///
    /// This will rewrite binary operators such as binop::Implies,
    /// binop::Equals, and binop::Xor, using only unop::Not, multop::Or,
    /// and multop::And.
    ///
    /// This visitor is public, because it's convenient
    /// to derive from it and override some of its methods.
    /// But if you just want the functionality, consider using
    /// spot::ltl::unabbreviate_logic instead.
    class unabbreviate_logic_visitor : public clone_visitor
    {
      typedef clone_visitor super;
    public:
      unabbreviate_logic_visitor();
      virtual ~unabbreviate_logic_visitor();

      using super::visit;
      void visit(binop* bo);

      virtual formula* recurse(formula* f);
    };

    /// \brief Clone rewrite a formula to remove most of the abbreviated
    /// logical operators.
    ///
    /// This will rewrite binary operators such as binop::Implies,
    /// binop::Equals, and binop::Xor, using only unop::Not, multop::Or,
    /// and multop::And.
    formula* unabbreviate_logic(formula* f);
    /// \brief Clone rewrite a formula to remove most of the abbreviated
    /// logical operators.
    const formula* unabbreviate_logic(const formula* f);

  }
}

#endif // SPOT_LTLVISIT_LUNABBREV_HH
