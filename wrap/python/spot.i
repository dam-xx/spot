%module spot

%include "std_string.i"
%include "std_list.i"

%import "buddy.i"

%{
#include <iostream>
#include <fstream>
#include <sstream>

#include "misc/version.hh"

#include "ltlast/formula.hh"
#include "ltlast/refformula.hh"
#include "ltlast/atomic_prop.hh"
#include "ltlast/binop.hh"
#include "ltlast/constant.hh"
#include "ltlast/multop.hh"
#include "ltlast/unop.hh"
#include "ltlast/visitor.hh"

#include "ltlenv/environment.hh"
#include "ltlenv/defaultenv.hh"

#include "ltlparse/public.hh"

#include "ltlvisit/clone.hh"
#include "ltlvisit/destroy.hh"
#include "ltlvisit/dotty.hh"
#include "ltlvisit/dump.hh"
#include "ltlvisit/lunabbrev.hh"
#include "ltlvisit/nenoform.hh"
#include "ltlvisit/tostring.hh"
#include "ltlvisit/tunabbrev.hh"

#include "tgba/bdddict.hh"
#include "tgba/bddprint.hh"
#include "tgba/state.hh"
#include "tgba/succiter.hh"
#include "tgba/tgba.hh"
#include "tgba/statebdd.hh"
#include "tgba/tgbabddcoredata.hh"
#include "tgba/succiterconcrete.hh"
#include "tgba/tgbabddconcrete.hh"
#include "tgba/tgbaexplicit.hh"
#include "tgba/tgbaproduct.hh"
#include "tgba/tgbatba.hh"

#include "tgbaalgos/ltl2tgba.hh"
#include "tgbaalgos/dotty.hh"
#include "tgbaalgos/lbtt.hh"
#include "tgbaalgos/magic.hh"
#include "tgbaalgos/save.hh"

using namespace spot::ltl;
using namespace spot;
%}

%include "misc/version.hh"

%include "ltlast/formula.hh"
%include "ltlast/refformula.hh"
%include "ltlast/atomic_prop.hh"
%include "ltlast/binop.hh"
%include "ltlast/constant.hh"
%include "ltlast/multop.hh"
%include "ltlast/unop.hh"
%include "ltlast/visitor.hh"

%include "ltlenv/environment.hh"
%include "ltlenv/defaultenv.hh"

%include "ltlparse/public.hh"

%include "ltlvisit/clone.hh"
%include "ltlvisit/destroy.hh"
%include "ltlvisit/dotty.hh"
%include "ltlvisit/dump.hh"
%include "ltlvisit/lunabbrev.hh"
%include "ltlvisit/nenoform.hh"
%include "ltlvisit/tostring.hh"
%include "ltlvisit/tunabbrev.hh"

%feature("new") spot::ltl_to_tgba;
%feature("new") spot::tgba::get_init_state;
%feature("new") spot::tgba::succ_iter;
%feature("new") spot::tgba_succ_iterator::current_state;

// Help SWIG with namespace lookups.
#define ltl spot::ltl
%include "tgba/bdddict.hh"
%include "tgba/bddprint.hh"
%include "tgba/state.hh"
%include "tgba/succiter.hh"
%include "tgba/tgba.hh"
%include "tgba/statebdd.hh"
%include "tgba/tgbabddcoredata.hh"
%include "tgba/succiterconcrete.hh"
%include "tgba/tgbabddconcrete.hh"
%include "tgba/tgbaexplicit.hh"
%include "tgba/tgbaproduct.hh"
%include "tgba/tgbatba.hh"

%include "tgbaalgos/ltl2tgba.hh"
%include "tgbaalgos/dotty.hh"
%include "tgbaalgos/lbtt.hh"
%include "tgbaalgos/magic.hh"
%include "tgbaalgos/save.hh"
#undef ltl

%extend spot::ltl::formula {

  // When comparing formula, make sure Python compare our
  // pointers, not the pointers to its wrappers.
  int
  __cmp__(const spot::ltl::formula* b)
  {
    return b - self;
  }

  std::string
  __str__(void)
  {
    return spot::ltl::to_string(self);
  }

}

%nodefault std::ostream;
namespace std {
  class ostream {};
  class ofstream : public ostream
  {
  public:
     ofstream(const char *fn);
     ~ofstream();
  };
  class ostringstream : public ostream
  {
  public:
     ostringstream();
     std::string str() const;
     ~ofstream();
  };
}

%inline %{

spot::ltl::parse_error_list
empty_parse_error_list()
{
  parse_error_list l;
  return l;
}

std::ostream&
get_cout()
{
  return std::cout;
}

std::ostream&
get_cerr()
{
  return std::cerr;
}

void
print_on(std::ostream& on, const std::string& what)
{
  on << what;
}

%}
