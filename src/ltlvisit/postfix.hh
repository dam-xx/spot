#ifndef SPOT_LTLVISIT_POSTFIX_HH
# define SPOT_LTLVISIT_POSTFIX_HH

#include "ltlast/formula.hh"
#include "ltlast/visitor.hh"

namespace spot
{
  namespace ltl
  {
    /// \brief Apply a algorithm on each node of an AST,
    /// during a postfix traversal.
    ///
    /// Override one or more of the postifix_visitor::doit methods
    /// with the algorithm to apply.
    class postfix_visitor : public visitor
    {
    public:
      postfix_visitor();
      virtual ~postfix_visitor();

      void visit(atomic_prop* ap);
      void visit(unop* uo);
      void visit(binop* bo);
      void visit(multop* mo);
      void visit(constant* c);

      virtual void doit(atomic_prop* ap);
      virtual void doit(unop* uo);
      virtual void doit(binop* bo);
      virtual void doit(multop* mo);
      virtual void doit(constant* c);
      virtual void doit_default(formula* f);
    };
  }
}

#endif // SPOT_LTLVISIT_POSTFIX_HH
