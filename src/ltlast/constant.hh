#ifndef SPOT_LTLAST_CONSTANT_HH
# define SPOT_LTLAST_CONSTANT_HH

#include "formula.hh"

namespace spot
{
  namespace ltl
  {

    /// A constant (True or False)
    class constant : public formula
    {
    public:
      enum type { False, True };

      constant(type val);
      virtual ~constant();

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      /// Return the value of the constant.
      type val() const;
      /// Return the value of the constant as a string.
      const char* val_name() const;

    private:
      type val_;
    };

  }
}

#endif // SPOT_LTLAST_CONSTANT_HH
