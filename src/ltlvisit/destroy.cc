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
    destroy(const formula* f)
    {
      destroy_visitor v;
      const_cast<formula*>(f)->accept(v);
    }
  }
}
