#ifndef SPOT_LTLAST_UNOP_HH
# define SPOT_LTLAST_UNOP_HH

#include <map>
#include "refformula.hh"

namespace spot
{
  namespace ltl
  {

    /// Unary operator.
    class unop : public ref_formula
    {
    public:
      enum type { Not, X, F, G };

      /// Build an unary operator with operation \a op and
      /// child \a child.
      static unop* instance(type op, formula* child);

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

      /// Number of instantiated unary operators.  For debugging.
      static unsigned instance_count();

    protected:
      typedef std::pair<type, formula*> pair;
      typedef std::map<pair, formula*> map;
      static map instances;

      unop(type op, formula* child);
      virtual ~unop();

    private:
      type op_;
      formula* child_;
    };

  }
}

#endif // SPOT_LTLAST_UNOP_HH
