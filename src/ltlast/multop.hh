#ifndef SPOT_LTLAST_MULTOP_HH
# define SPOT_LTLAST_MULTOP_HH

#include <vector>
#include "formulae.hh"

namespace spot
{
  namespace ltl
  {
    
    class multop : public formulae
    {
    public:
      enum type { Or, And };

      // A multop has at least two arguments.
      multop(type op, formulae* first, formulae* second);
      // More argument can be added.
      void add(formulae* f);
      
      virtual ~multop();

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      unsigned size() const;
      const formulae* nth(unsigned n) const;
      formulae* nth(unsigned n);

      type op() const;
      const char* op_name() const;

    private:
      type op_;
      std::vector<formulae*> children_;
    };

  }
}

#endif // SPOT_LTLAST_MULTOP_HH
