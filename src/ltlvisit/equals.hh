#include "ltlast/formula.hh"
#include "ltlast/visitor.hh"

namespace spot 
{
  namespace ltl
  {
    /// \brief Check for equality between two formulae.
    ///
    /// This visitor is public, because it's convenient
    /// to derive from it and override some of its methods.
    /// But if you just want the functionality, consider using
    /// spot::ltl::equals instead.
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
    
    /// \brief Check whether two formulae are syntaxically equal.
    /// \return \c true iff \a f1 equals \a f2.
    ///
    /// This tests for syntaxic equality rather than semantic equality.
    /// Two formulae are equals of their abstract syntax tree are equals.
    /// ltl::multop children can be permuted or repeated without
    /// impact on the result of this comparison.
    bool equals(const formula* f1, const formula* f2);
  }
}

