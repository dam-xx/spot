%module spot

%include "std_string.i"
%include "std_list.i"

%{
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

using namespace spot::ltl;
%}

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
