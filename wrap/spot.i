%module spot

%include "std_string.i"
%include "std_list.i"

%{
#include "ltlenv/environment.hh"
#include "ltlenv/defaultenv.hh"
#include "ltlast/formula.hh"
#include "ltlparse/public.hh"
#include "ltlast/visitor.hh"

#include "ltlvisit/clone.hh"
#include "ltlvisit/dotty.hh"
#include "ltlvisit/dump.hh"
#include "ltlvisit/equals.hh"
#include "ltlvisit/lunabbrev.hh"
#include "ltlvisit/tunabbrev.hh"
#include "ltlvisit/nenoform.hh"
#include "ltlvisit/tostring.hh"

using namespace spot::ltl;
%}

%include "ltlenv/environment.hh"
%include "ltlenv/defaultenv.hh"
%include "ltlast/formula.hh"
%include "ltlparse/public.hh"
%include "ltlast/visitor.hh"

%include "ltlvisit/clone.hh"
%include "ltlvisit/dotty.hh"
%include "ltlvisit/dump.hh"
%include "ltlvisit/equals.hh"
%include "ltlvisit/lunabbrev.hh"
%include "ltlvisit/tunabbrev.hh"
%include "ltlvisit/nenoform.hh"
%include "ltlvisit/tostring.hh"

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
