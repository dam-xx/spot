#ifndef SPOT_LTLAST_CONSTANT_HH
# define SPOT_LTLAST_CONSTANT_HH

#include "formula.hh"

namespace spot
{
  namespace ltl
  {
    
    class constant : public formula
    {
    public:
      enum type { False, True };

      constant(type val);
      virtual ~constant();

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      const formula* child() const;
      formula* child();

      type val() const;
      const char* val_name() const;

    private:
      type val_;
    };

  }
}

#endif // SPOT_LTLAST_CONSTANT_HH
