#ifndef SPOT_LTLAST_VISITOR_HH
# define SPOT_LTLAST_VISITOR_HH

#include "predecl.hh"

namespace spot {
  namespace ltl {
    
    /// \brief Formula visitor that can modify the formula.
    ///
    /// Writing visitors is the prefered way 
    /// to traverse a formula, since it doesn't
    /// involve any cast.
    ///
    /// If you do not need to modify the visited formula, inherit from
    /// spot::ltl:const_visitor instead.
    struct visitor 
    {
      virtual void visit(atomic_prop* node) = 0;
      virtual void visit(constant* node) = 0;
      virtual void visit(binop* node) = 0;
      virtual void visit(unop* node) = 0;
      virtual void visit(multop* node) = 0;
    };

    /// \brief Formula visitor that cannot modify the formula.
    ///
    /// Writing visitors is the prefered way 
    /// to traverse a formula, since it doesn't
    /// involve any cast.
    ///
    /// If you want to modify the visited formula, inherit from
    /// spot::ltl:visitor instead.
    struct const_visitor 
    {
      virtual void visit(const atomic_prop* node) = 0;
      virtual void visit(const constant* node) = 0;
      virtual void visit(const binop* node) = 0;
      virtual void visit(const unop* node) = 0;
      virtual void visit(const multop* node) = 0;
    };


  }
}

#endif // SPOT_LTLAST_VISITOR_HH
