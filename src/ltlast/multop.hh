#ifndef SPOT_LTLAST_MULTOP_HH
# define SPOT_LTLAST_MULTOP_HH

#include <vector>
#include "formula.hh"

namespace spot
{
  namespace ltl
  {
    
    class multop : public formula
    {
    public:
      enum type { Or, And };

      // A multop has at least two arguments.
      multop(type op, formula* first, formula* second);
      // More argument can be added.
      void add(formula* f);
      
      virtual ~multop();

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      unsigned size() const;
      const formula* nth(unsigned n) const;
      formula* nth(unsigned n);

      type op() const;
      const char* op_name() const;

    private:
      type op_;
      std::vector<formula*> children_;
    };

  }
}

#endif // SPOT_LTLAST_MULTOP_HH
