#include "ltlast/formula.hh"
#include "ltlast/visitor.hh"

namespace spot 
{
  namespace ltl
  {
    // This visitor is public, because it's convenient
    // to derive from it and override part of its methods.
    class equals_visitor : public const_visitor
    {
    public:
      equals_visitor(const formula* f);
      virtual ~equals_visitor();

      // Return true iff the visitor has visited a
      // formula which is equal to `f'.
      bool result() const;
      
      void visit(const atomic_prop* ap);
      void visit(const unop* uo);
      void visit(const binop* bo);
      void visit(const multop* bo);
      void visit(const constant* c);
	
    private:
      const formula* f_;
      bool result_;
	      
    };

    bool equals(const formula* f1, const formula* f2);
  }
}
