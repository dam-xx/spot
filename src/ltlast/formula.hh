#ifndef SPOT_LTLAST_FORMULAE_HH
# define SPOT_LTLAST_FORMULAE_HH

#include "predecl.hh"

namespace spot
{
  namespace ltl
  {

    /// \brief An LTL formula.
    ///
    /// The only way you can work with a formula is to
    /// build a spot::ltl::visitor or spot::ltl::const_visitor.
    class formula
    {
    public:
      virtual ~formula();

      /// Entry point for vspot::ltl::visitor instances.
      virtual void accept(visitor& v) = 0;
      /// Entry point for vspot::ltl::const_visitor instances.
      virtual void accept(const_visitor& v) const = 0;

      /// \brief clone this node
      ///
      /// This increments the reference counter of this node (if one is
      /// used).  You should almost never use this method directly as
      /// it doesn't touch the children.  If you want to clone a
      /// whole formula, use spot::ltl::clone() instead.
      formula* ref();
      /// \brief release this node
      ///
      /// This decrements the reference counter of this node (if one is
      /// used) and can free the object.  You should almost never use
      /// this method directly as it doesn't touch the children.  If you
      /// want to release a whole formula, use spot::ltl::destroy() instead.
      static void unref(formula* f);

    protected:
      /// \brief increment reference counter if any
      virtual void ref_();
      /// \brief decrement reference counter if any, return true when
      /// the instance must be deleted (usually when the counter hits 0).
      virtual bool unref_();
    };

  }
}



#endif // SPOT_LTLAST_FORMULAE_HH
