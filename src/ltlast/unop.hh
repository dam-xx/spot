#ifndef SPOT_LTLAST_UNOP_HH
# define SPOT_LTLAST_UNOP_HH

#include "formulae.hh"

namespace spot
{
  namespace ltl
  {
    
    class unop : public formulae
    {
    public:
      enum type { Not, X, F, G };

      unop(type op, formulae* child);
      virtual ~unop();

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      virtual bool equals(const formulae* h) const;

      const formulae* child() const;
      formulae* child();

      type op() const;
      const char* op_name() const;

    private:
      type op_;
      formulae* child_;
    };

  }
}

#endif // SPOT_LTLAST_UNOP_HH
