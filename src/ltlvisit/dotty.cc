#include "dotty.hh"
#include "ltlast/visitor.hh"
#include "ltlast/allnodes.hh"

namespace spot
{
  namespace ltl
  {

    class dotty_visitor : public const_visitor
    {
    public:
      typedef std::map<const formula*, int> map;
      dotty_visitor(std::ostream& os, map& m)
	: os_(os), father_(-1), node_(m)
      {
      }

      virtual
      ~dotty_visitor()
      {
      }

      void
      visit(const atomic_prop* ap)
      {
	draw_node_(ap, ap->name(), true);
      }

      void
      visit(const constant* c)
      {
	draw_node_(c, c->val_name(), true);
      }

      void
      visit(const binop* bo)
      {
	if (draw_node_(bo, bo->op_name()))
	  {
	    dotty_visitor v(*this);
	    bo->first()->accept(v);
	    bo->second()->accept(*this);
	  }
      }

      void
      visit(const unop* uo)
      {
	if (draw_node_(uo, uo->op_name()))
	  uo->child()->accept(*this);
      }

      void
      visit(const multop* mo)
      {
	if (!draw_node_(mo, mo->op_name()))
	  return;
	unsigned max = mo->size();
	for (unsigned n = 0; n < max; ++n)
	  {
	    dotty_visitor v(*this);
	    mo->nth(n)->accept(v);
	  }
      }
    private:
      std::ostream& os_;
      int father_;
      map& node_;

      bool
      draw_node_(const formula* f, const std::string& str, bool rec = false)
      {
	map::iterator i = node_.find(f);
	int node;
	bool node_exists = false;
	if (i != node_.end())
	  {
	    node = i->second;
	    node_exists = true;
	  }
	else
	  {
	    node = node_.size();
	    node_[f] = node;
	  }
	// the link
	if (father_ >= 0)
	  os_ << "  " << father_ << " -> " << node << ";" << std::endl;
	father_ = node;
	// the node
	if (node_exists)
	  return false;
	os_ << "  " << node
	    << " [label=\"" << str << "\"";
	if (rec)
	  os_ << ", shape=box";
	os_ << "];" << std::endl;
	return true;
      }
    };

    std::ostream&
    dotty(std::ostream& os, const formula* f)
    {
      dotty_visitor::map m;
      dotty_visitor v(os, m);
      os << "digraph G {" << std::endl;
      f->accept(v);
      os << "}" << std::endl;
      return os;
    }

  }
}
