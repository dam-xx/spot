#ifndef SPOT_LTLAST_BINOP_HH
# define SPOT_LTLAST_BINOP_HH

#include "formulae.hh"

namespace spot
{
  namespace ltl
  {
    
    class binop : public formulae
    {
    public:
      // And and Or are not here.  Because they
      // are often nested we represent them as multops.
      enum type { Xor, Implies, Equiv, U, R };

      binop(type op, formulae* first, formulae* second);
      virtual ~binop();

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      const formulae* first() const;
      formulae* first();
      const formulae* second() const;
      formulae* second();

      type op() const;
      const char* op_name() const;

    private:
      type op_;
      formulae* first_;
      formulae* second_;
    };

  }
}

#endif // SPOT_LTLAST_BINOP_HH
