#ifndef SPOT_LTLAST_CONSTANT_HH
# define SPOT_LTLAST_CONSTANT_HH

#include "formulae.hh"

namespace spot
{
  namespace ltl
  {
    
    class constant : public formulae
    {
    public:
      enum type { False, True };

      constant(type val);
      virtual ~constant();

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      virtual bool equals(const formulae* h) const;

      const formulae* child() const;
      formulae* child();

      type val() const;
      const char* val_name() const;

    private:
      type val_;
    };

  }
}

#endif // SPOT_LTLAST_CONSTANT_HH
