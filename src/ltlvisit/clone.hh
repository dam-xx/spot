#ifndef SPOT_LTLVISIT_CLONE_HH
# define SPOT_LTLVISIT_CLONE_HH

#include "ltlast/formula.hh"
#include "ltlast/visitor.hh"

namespace spot
{
  namespace ltl
  {
    /// \brief Clone a formula.
    ///
    /// This visitor is public, because it's convenient
    /// to derive from it and override part of its methods.
    /// But if you just want the functionality, consider using
    /// spot::ltl::clone instead.
    class clone_visitor : public visitor
    {
    public:
      clone_visitor();
      virtual ~clone_visitor();

      formula* result() const;

      void visit(atomic_prop* ap);
      void visit(unop* uo);
      void visit(binop* bo);
      void visit(multop* mo);
      void visit(constant* c);

      virtual formula* recurse(formula* f);

    protected:
      formula* result_;
    };

    /// \brief Clone a formula.
    formula* clone(formula* f);
    /// \brief Clone a formula.
    formula* clone(const formula* f);
  }
}

#endif // SPOT_LTLVISIT_LUNABBREV_HH
