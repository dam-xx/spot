#include "dotty.hh"
#include "ltlast/visitor.hh"
#include "ltlast/allnodes.hh"


namespace spot 
{
  namespace ltl
  {

    class dotty_visitor : public spot::ltl::const_visitor
    {
    public:
      dotty_visitor(std::ostream& os = std::cout)
	: os_(os), label_("i")
      {
      }

      virtual 
      ~dotty_visitor()
      {
      }

      void 
      visit(const spot::ltl::atomic_prop* ap)
      {
	draw_node_(ap->name());
      }

      void
      visit(const spot::ltl::constant* c)
      {
	draw_node_(c->val_name());
      }
      
      void
      visit(const spot::ltl::binop* bo)
      {
	draw_rec_node_(bo->op_name());
	std::string label = label_;
	
	label_ += "l";
	draw_link_(label, label_);
	bo->first()->accept(*this);
	label_ = draw_link_(label, label + "r");
	bo->second()->accept(*this);
      }
      
      void
      visit(const spot::ltl::unop* uo)
      {
	draw_rec_node_(uo->op_name());
	label_ = draw_link_(label_, label_ + "c");
	uo->child()->accept(*this);
      }
      
      void
      visit(const spot::ltl::multop* mo)
      {
	draw_rec_node_(mo->op_name());

	unsigned max = mo->size();
	std::string label = label_;
	for (unsigned n = 0; n < max; ++n)
	  {
	    // FIXME: use `n' as a string for label names.
	    label_ = draw_link_(label, label_ + "m");
	    mo->nth(n)->accept(*this);
	  }
      }
    private:
      std::ostream& os_;
      std::string label_;

      const std::string&
      draw_link_(const std::string& in, const std::string& out)
      {
	os_ << "  " << in << " -> " << out << ";" << std::endl;
	return out;
      }

      void 
      draw_rec_node_(const char* str) const
      {
	os_ << "  " << label_ << " [label=\"" << str << "\", shabe=box];" 
	    << std::endl;
      }

      void
      draw_node_(const std::string& str) const
      {
	os_ << "  " << label_ << " [label=\"" << str << "\"];" << std::endl;
      }
      
    };

    void 
    dotty(const formula& f, std::ostream& os)
    {
      dotty_visitor v(os);
      os << "digraph G {" << std::endl;
      f.accept(v);
      os << "}" << std::endl;
    }

  }
}
