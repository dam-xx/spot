#include <cassert>
#include <sstream>
#include "tostring.hh"
#include "ltlast/visitor.hh"
#include "ltlast/allnodes.hh"


namespace spot 
{
  namespace ltl
  {

    class to_string_visitor : public const_visitor
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
      visit(const atomic_prop* ap)
      {
	os_ << ap->name();
      }

      void
      visit(const constant* c)
      {
	os_ << c->val_name();
      }      
      
      void
      visit(const binop* bo)
      {
	os_ << "(";
	bo->first()->accept(*this);
	
	switch(bo->op())
	  {
	  case binop::Xor:
	    os_ << " ^ ";
	    break;
	  case binop::Implies:
	    os_ << " => ";
	    break;
	  case binop::Equiv:
	    os_ << " <=> ";
	    break;
	  case binop::U:
	    os_ << " U ";
	    break;
	  case binop::R:
	    os_ << " R ";
	    break;
	  }
	
	bo->second()->accept(*this);
	os_ << ")";
      }
      
      void
      visit(const unop* uo)
      {   
	switch(uo->op())
	  {
	  case unop::Not:
	    os_ << "!";
	    break;
	  case unop::X:
	    os_ << "X";
	    break;
	  case unop::F:
	    os_ << "F";
	    break;
	  case unop::G:
	    os_ << "G";
	    break;
	  }
        
	uo->child()->accept(*this);
      }
      
      void
      visit(const multop* mo)
      {
	os_ << "(";
	unsigned max = mo->size();
	mo->nth(0)->accept(*this);
	const char* ch = " ";
	switch (mo->op())
	  {
	  case multop::Or:
	    ch = " | ";
	    break; 
	  case multop::And:
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

    std::string 
    to_string(const formula& f)
    {
      std::ostringstream os;
      to_string(f, os);
      return os.str();
    }
  }
}
