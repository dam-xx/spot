#ifndef SPOT_LTLAST_VISITOR_HH
# define SPOT_LTLAST_VISITOR_HH

#include "predecl.hh"
#include "misc/const_sel.hh"

namespace spot {
  namespace ltl {
    
    template <bool WantConst>
    struct generic_visitor
    {
      virtual void visit(typename const_sel<atomic_prop, WantConst>::t* node) 
	= 0;

      virtual void visit(typename const_sel<constant, WantConst>::t* node) 
	= 0;

      virtual void visit(typename const_sel<binop, WantConst>::t* node) = 0;
      
      virtual void visit(typename const_sel<unop, WantConst>::t* node) = 0;

      virtual void visit(typename const_sel<multop, WantConst>::t* node) = 0;
    };

    struct visitor : public generic_visitor<false> {};
    struct const_visitor : public generic_visitor<true> {};

  }
}

#endif // SPOT_LTLAST_VISITOR_HH
