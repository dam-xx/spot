#ifndef SPOT_LTLAST_MULTOP_HH
# define SPOT_LTLAST_MULTOP_HH

#include <vector>
#include <map>
#include "refformula.hh"

namespace spot
{
  namespace ltl
  {

    /// \brief Multi-operand operators.
    ///
    /// These operators are considered commutative and associative.
    class multop : public ref_formula
    {
    public:
      enum type { Or, And };


      /// \brief Build a spot::ltl::multop with no child.
      ///
      /// This has little value unless you call multop::add later.
      static multop* instance(type op);

      /// \brief Build a spot::ltl::multop with two children.
      ///
      /// If one of the children itself is a spot::ltl::multop
      /// with the same type, it will be merged.  I.e., children
      /// if that child will be added, and that child itself will
      /// be destroyed.
      static multop* instance(type op, formula* first, formula* second);

      /// \brief Add another child to this operator.
      ///
      /// If \a f itself is a spot::ltl::multop with the same type, it
      /// will be merged.  I.e., children of \a f will be added, and
      /// that \a f will will be destroyed.
      ///
      /// Note that this function overwrites the supplied ltl::multop pointer.
      /// The old value is released and should not be used after this.
      static void add(multop** m, formula* f);

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

      /// Number of instantiated multi-operand operators.  For debugging.
      static unsigned instance_count();

    protected:
      // Sorted list of formulae.  (Sorted by pointer comparison.)
      typedef std::vector<formula*> vec;


      typedef std::pair<type, vec*> pair;
      /// Comparison functor used internally by ltl::multop.
      struct paircmp
      {
	bool
	operator () (const pair& p1, const pair& p2) const
	{
	  if (p1.first != p2.first)
	    return p1.first < p2.first;
	  return *p1.second < *p2.second;
	}
      };
      typedef std::map<pair, formula*, paircmp> map;
      static map instances;

      multop(type op, vec* v);
      static multop* instance(type op, vec* v);
      static vec* add(type op, vec* v, formula* f);
      static void add_sorted(vec* v, formula* f);

      virtual ~multop();

    private:
      type op_;
      vec* children_;
    };

  }
}

#endif // SPOT_LTLAST_MULTOP_HH
