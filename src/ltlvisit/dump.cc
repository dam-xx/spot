#include "dump.hh"
#include "ltlast/visitor.hh"
#include "ltlast/allnodes.hh"


namespace spot 
{
  namespace ltl
  {

    class dump_visitor : public spot::ltl::const_visitor
    {
    public:
      dump_visitor(std::ostream& os = std::cout)
	: os_(os)
      {
      }
      
      virtual 
      ~dump_visitor()
      {
      }

      void 
      visit(const spot::ltl::atomic_prop* ap)
      {
	os_ << "AP(" << ap->name() << ")";
      }

      void
      visit(const spot::ltl::constant* c)
      {
	os_ << "constant(" << c->val_name() << ")";
      }      
      
      void
      visit(const spot::ltl::binop* bo)
      {
	os_ << "binop(" << bo->op_name() << ", ";
	bo->first()->accept(*this);
	os_ << ", ";
	bo->second()->accept(*this);
	os_ << ")";
      }
      
      void
      visit(const spot::ltl::unop* uo)
      {
	os_ << "unop(" << uo->op_name() << ", ";
	uo->child()->accept(*this);
	os_ << ")";
      }
      
      void
      visit(const spot::ltl::multop* mo)
      {
	os_ << "multop(" << mo->op_name() << ", ";
	unsigned max = mo->size();
	mo->nth(0)->accept(*this);
	for (unsigned n = 1; n < max; ++n)
	  {
	    std::cout << ", ";
	    mo->nth(n)->accept(*this);
	  }
	os_ << ")";
      }
    private:
      std::ostream& os_;
    };

    void 
    dump(const formulae& f, std::ostream& os)
    {
      dump_visitor v(os);
      f.accept(v);
    }

  }
}
