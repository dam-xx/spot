#ifndef SPOT_LTLAST_UNOP_HH
# define SPOT_LTLAST_UNOP_HH

#include "formula.hh"

namespace spot
{
  namespace ltl
  {
    
    class unop : public formula
    {
    public:
      enum type { Not, X, F, G };

      unop(type op, formula* child);
      virtual ~unop();

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      const formula* child() const;
      formula* child();

      type op() const;
      const char* op_name() const;

    private:
      type op_;
      formula* child_;
    };

  }
}

#endif // SPOT_LTLAST_UNOP_HH
