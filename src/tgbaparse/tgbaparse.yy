%{
#include <string>
#include "public.hh"
%}

%parse-param {spot::tgba_parse_error_list &error_list}
%parse-param {spot::ltl::environment &parse_environment}
%parse-param {spot::tgba_explicit* &result}
%debug
%error-verbose
%union
{
  int token;
  std::string* str;
  std::list<std::pair<bool, spot::ltl::formula*> >* list;
}

%{
/* tgbaparse.hh and parsedecl.hh include each other recursively.
   We mut ensure that YYSTYPE is declared (by the above %union)
   before parsedecl.hh uses it. */
#include "parsedecl.hh"
using namespace spot::ltl;

/* Ugly hack so that Bison use tgbayylex, not yylex.
   (%name-prefix doesn't work for the lalr1.cc skeleton
   at the time of writing.)  */
#define yylex tgbayylex

typedef std::pair<bool, spot::ltl::formula*> pair;
%}

%token <str> STRING
%token <str> IDENT
%type <str> strident
%type <list> prop_list


%%
lines:
       | lines line
       ;

line: strident ',' strident ',' prop_list ',' prop_list ';'
       {
	 spot::tgba_explicit::transition* t
	   = result->create_transition(*$1, *$3);
	 std::list<pair>::iterator i;
	 for (i = $5->begin(); i != $5->end(); ++i)
	   if (i->first)
	     result->add_neg_condition(t, i->second);
	   else
	     result->add_condition(t, i->second);
	 for (i = $7->begin(); i != $7->end(); ++i)
	   if (i->first)
	     result->add_neg_promise(t, i->second);
	   else
	     result->add_promise(t, i->second);
	 delete $1;
	 delete $3;
	 delete $5;
	 delete $7;
       }
       ;

strident: STRING | IDENT;

prop_list:
       {
	 $$ = new std::list<pair>;
       }
       | prop_list strident
       {
	 if (*$2 != "")
	   $1->push_back(pair(false, parse_environment.require(*$2)));
	 delete $2;
	 $$ = $1;
       }
       | prop_list '!' strident
       {
	 if (*$3 != "")
	   $1->push_back(pair(true, parse_environment.require(*$3)));
	 delete $3;
	 $$ = $1;
       }
       ;

;

%%

void
yy::Parser::print_()
{
  if (looka_ == STRING || looka_ == IDENT)
    YYCDEBUG << " '" << *value.str << "'";
}

void
yy::Parser::error_()
{
  error_list.push_back(spot::tgba_parse_error(location, message));
}

namespace spot
{
  tgba_explicit*
  tgba_parse(const std::string& name,
	     tgba_parse_error_list& error_list,
	     environment& env,
	     bool debug)
  {
    if (tgbayyopen(name))
	{
	  error_list.push_back
	    (tgba_parse_error(yy::Location(),
			       std::string("Cannot open file ") + name));
	  return 0;
	}
      tgba_explicit* result = new tgba_explicit;
      tgbayy::Parser parser(debug, yy::Location(), error_list, env, result);
      parser.parse();
      tgbayyclose();
      return result;
    }
}

// Local Variables:
// mode: c++
// End:
