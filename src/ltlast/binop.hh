#ifndef SPOT_LTLAST_BINOP_HH
# define SPOT_LTLAST_BINOP_HH

#include "formula.hh"

namespace spot
{
  namespace ltl
  {
    
    class binop : public formula
    {
    public:
      // And and Or are not here.  Because they
      // are often nested we represent them as multops.
      enum type { Xor, Implies, Equiv, U, R };

      binop(type op, formula* first, formula* second);
      virtual ~binop();

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      const formula* first() const;
      formula* first();
      const formula* second() const;
      formula* second();

      type op() const;
      const char* op_name() const;

    private:
      type op_;
      formula* first_;
      formula* second_;
    };

  }
}

#endif // SPOT_LTLAST_BINOP_HH
