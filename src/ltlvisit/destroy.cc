#include "ltlvisit/destroy.hh"

namespace spot
{
  namespace ltl
  {

    class destroy_visitor : public postfix_visitor
    {
    public:
      virtual void
      doit_default(formula* c)
      {
	formula::unref(c);
      }
    };

    void
    destroy(formula* f)
    {
      destroy_visitor v;
      f->accept(v);
    }

    void
    destroy(const formula* f)
    {
      destroy(const_cast<formula*>(f));
    }
  }
}
