#include <cassert>
#include "tostring.hh"
#include "ltlast/visitor.hh"
#include "ltlast/allnodes.hh"


namespace spot 
{
  namespace ltl
  {

    class to_string_visitor : public spot::ltl::const_visitor
    {
    public:
      to_string_visitor(std::ostream& os = std::cout)
	: os_(os)
      {
      }
      
      virtual 
      ~to_string_visitor()
      {
      }

      void 
      visit(const spot::ltl::atomic_prop* ap)
      {
	   os_ << ap->name();
    
      }

      void
      visit(const spot::ltl::constant* c)
      {
	os_ << c->val_name();
      }      
      
      void
      visit(const spot::ltl::binop* bo)
      {
	os_ << "(";
	bo->first()->accept(*this);
	
	switch(bo->op())
	   {
	      case spot::ltl::binop::Xor:
	        os_ << " ^ ";
	      break;
	      case spot::ltl::binop::Implies:
	        os_ << " => ";
	      break;
	      case spot::ltl::binop::Equiv:
	        os_ << " <=> ";
	      break;
	      case spot::ltl::binop::U:
	        os_ << " U ";
	      break;
	      case spot::ltl::binop::R:
	        os_ << " R ";
	      break;
	   }
	
	bo->second()->accept(*this);
	os_ << ")";
      }
      
      void
      visit(const spot::ltl::unop* uo)
      {   
 	 switch(uo->op())
	   {
	   case spot::ltl::unop::Not:
	     os_ << "!";
	     break;
	   case spot::ltl::unop::X:
	     os_ << "X";
	     break;
	   case spot::ltl::unop::F:
	     os_ << "F";
	     break;
	   case spot::ltl::unop::G:
	     os_ << "G";
	       break;
	   }
        
	uo->child()->accept(*this);
      }
      
      void
      visit(const spot::ltl::multop* mo)
      {
	os_ << "(";
	unsigned max = mo->size();
	mo->nth(0)->accept(*this);
	 const char* ch = " ";
	switch (mo->op())
	       {
	          case spot::ltl::multop::Or:
	            ch = " | ";
	          break; 
	          case spot::ltl::multop::And:
	            ch = " & ";
	          break;
	       }

	for (unsigned n = 1; n < max; ++n)
	  {
	    os_ << ch;
	    mo->nth(n)->accept(*this);
	  }
	os_ << ")";
      }
    private:
      std::ostream& os_;
    };

    void 
    to_string(const formula& f, std::ostream& os)
    {
      to_string_visitor v(os);
      f.accept(v);
    }

  }
}
