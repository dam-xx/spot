#ifndef SPOT_LTLAST_BINOP_HH
# define SPOT_LTLAST_BINOP_HH

#include <map>
#include "formula.hh"

namespace spot
{
  namespace ltl
  {

    /// Binary operator.
    class binop : public formula
    {
    public:
      /// Different kinds of binary opertaors
      ///
      /// And and Or are not here.  Because they
      /// are often nested we represent them as multops.
      enum type { Xor, Implies, Equiv, U, R };

      /// Build an unary operator with operation \a op and
      /// children \a first and \a second.
      static binop* instance(type op, formula* first, formula* second);

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      /// Get the first operand.
      const formula* first() const;
      /// Get the first operand.
      formula* first();
      /// Get the second operand.
      const formula* second() const;
      /// Get the second operand.
      formula* second();

      /// Get the type of this operator.
      type op() const;
      /// Get the type of this operator, as a string.
      const char* op_name() const;

    protected:
      typedef std::pair<formula*, formula*> pairf;
      typedef std::pair<type, pairf> pair;
      typedef std::map<pair, formula*> map;
      static map instances;

      binop(type op, formula* first, formula* second);
      virtual ~binop();

    private:
      type op_;
      formula* first_;
      formula* second_;
    };

  }
}

#endif // SPOT_LTLAST_BINOP_HH
