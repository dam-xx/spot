#ifndef SPOT_LTLVISIT_CLONE_HH
# define SPOT_LTLVISIT_CLONE_HH

#include "ltlast/formula.hh"
#include "ltlast/visitor.hh"

namespace spot 
{
  namespace ltl
  {
    // This visitor is public, because it's convenient
    // to derive from it and override part of its methods.
    class clone_visitor : public const_visitor
    {
    public:
      clone_visitor();
      virtual ~clone_visitor();

      formula* result() const;
      
      void visit(const atomic_prop* ap);
      void visit(const unop* uo);
      void visit(const binop* bo);
      void visit(const multop* mo);
      void visit(const constant* c);
	
      virtual formula* recurse(const formula* f);
      
    protected:
      formula* result_;
    };

    formula* clone(const formula* f);
  }
}

#endif // SPOT_LTLVISIT_LUNABBREV_HH
