#include "ltlast/formulae.hh"
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
      equals_visitor(const formulae* f);
      virtual ~equals_visitor();

      // Return true iff the visitor has visited a
      // formulae which is equal to `f'.
      bool result() const;
      
      void visit(const atomic_prop* ap);
      void visit(const unop* uo);
      void visit(const binop* bo);
      void visit(const multop* bo);
      void visit(const constant* c);
	
    private:
      const formulae* f_;
      bool result_;
	      
    };

    bool equals(const formulae* f1, const formulae* f2);
  }
}
