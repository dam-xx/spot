#ifndef SPOT_LTLAST_MULTOP_HH
# define SPOT_LTLAST_MULTOP_HH

#include <vector>
#include "formula.hh"

namespace spot
{
  namespace ltl
  {
    
    /// \brief Multi-operand operators.
    ///
    /// These operators are considered commutative and associative.
    class multop : public formula
    {
    public:
      enum type { Or, And };

      /// \brief Build a spot::ltl::multop with no child.
      ///
      /// This has little value unless you call multop::add later.
      multop(type op);
      /// \brief Build a spot::ltl::multop with two children.
      /// 
      /// If one of the children itself is a spot::ltl::multop
      /// with the same type, it will be merged.  I.e., children
      /// if that child will be added, and that child itself will
      /// be destroyed.
      multop(type op, formula* first, formula* second);
      /// \brief Add another child to this operator.
      ///
      /// If \a f itself is a spot::ltl::multop with the same type, it
      /// will be merged.  I.e., children of \a f will be added, and
      /// that \a f will will be destroyed.
      void add(formula* f);
      
      virtual ~multop();

      virtual void accept(visitor& v);
      virtual void accept(const_visitor& v) const;

      /// Get the number of children.
      unsigned size() const;
      /// \brief Get the nth children.
      ///
      /// Starting with \a n = 0.
      const formula* nth(unsigned n) const;
      /// \brief Get the nth children.
      ///
      /// Starting with \a n = 0.
      formula* nth(unsigned n);

      /// Get the type of this operator.
      type op() const;
      /// Get the type of this operator, as a string.
      const char* op_name() const;

    private:
      type op_;
      std::vector<formula*> children_;
    };

  }
}

#endif // SPOT_LTLAST_MULTOP_HH
