#ifndef SPOT_LTLAST_UNOP_HH
# define SPOT_LTLAST_UNOP_HH

#include "formula.hh"

namespace spot
{
  namespace ltl
  {
    
    /// Unary operator.
    class unop : public formula
    {
    public:
      enum type { Not, X, F, G };

      unop(type op, formula* child);
      virtual ~unop();

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      /// Get the sole operand of this operator.
      const formula* child() const;
      /// Get the sole operand of this operator.
      formula* child();

      /// Get the type of this operator.
      type op() const;
      /// Get the type of this operator, as a string.
      const char* op_name() const;

    private:
      type op_;
      formula* child_;
    };

  }
}

#endif // SPOT_LTLAST_UNOP_HH
