%{
#include <string>
#include "public.hh"
#include "ltlast/allnodes.hh"

extern spot::ltl::formula* result;

%}

%debug
%error-verbose
%union
{
  int token;
  std::string* str;
  spot::ltl::formula* ltl;
}

%{
/* Spotparse.hh and parsedecl.hh include each other recursively.
   We mut ensure that YYSTYPE is declared (by the above %union)
   before parsedecl.hh uses it. */
#include "parsedecl.hh"
using namespace spot::ltl;

// At the time of writing C++ support in Bison is quite
// new and there is still no way to pass error_list to
// the parser.  We use a global variable instead.
namespace spot
{
  namespace ltl
  {
    extern parse_error_list* error_list;
  }
}
%}

/* Logical operators.  */
%left <token> OP_AND OP_XOR OP_OR
%left <token> OP_IMPLIES OP_EQUIV

/* LTL operators.  */
%left <token> OP_U OP_R
%nonassoc <token> OP_F OP_G
%nonassoc <token> OP_X

/* Not has the most important priority.  */
%nonassoc <token> OP_NOT

/* Grouping (parentheses).  */
%token <token> PAR_OPEN PAR_CLOSE

/* Atomic proposition.  */
%token <str> ATOMIC_PROP

/* Constants */
%token CONST_TRUE 
%token CONST_FALSE
%token END_OF_INPUT

%type <ltl> result ltl_formula subformula

%%
result:       ltl_formula END_OF_INPUT
              { result = $$ = $1; 
		YYACCEPT;
	      }
	    | many_errors END_OF_INPUT
              { error_list->push_back(parse_error(@1,
				      "couldn't parse anything sensible")); 
	        result = $$ = 0;
		YYABORT;
	      }
	    | END_OF_INPUT
              { error_list->push_back(parse_error(@1,
				      "empty input")); 
	        result = $$ = 0;
		YYABORT;
	      }

many_errors_diagnosed : many_errors 
              { error_list->push_back(parse_error(@1, 
				     "unexpected input ignored")); }

ltl_formula: subformula
              { $$ = $1; }
	    | many_errors_diagnosed subformula
              { $$ = $2; }
	    | ltl_formula many_errors_diagnosed
              { $$ = $1; }

many_errors: error
	    | many_errors error

subformula: ATOMIC_PROP
	      { $$ = new atomic_prop(*$1); delete $1; }
	    | CONST_TRUE
              { $$ = new constant(constant::True); }
	    | CONST_FALSE
              { $$ = new constant(constant::False); }
	    | PAR_OPEN subformula PAR_CLOSE
	      { $$ = $2; }
	    | PAR_OPEN error PAR_CLOSE
              { error_list->push_back(parse_error(@$, 
                 "treating this parenthetical block as false")); 
	        $$ = new constant(constant::False);
	      }
	    | PAR_OPEN subformula many_errors PAR_CLOSE
              { error_list->push_back(parse_error(@3, 
				      "unexpected input ignored")); 
	        $$ = $2;
	      }
	    | OP_NOT subformula
	      { $$ = new unop(unop::Not, $2); }
            | subformula OP_AND subformula
	      { $$ = new multop(multop::And, $1, $3); }
	    | subformula OP_OR subformula
	      { $$ = new multop(multop::Or, $1, $3); }
	    | subformula OP_XOR subformula
	      { $$ = new binop(binop::Xor, $1, $3); }
	    | subformula OP_IMPLIES subformula
	      { $$ = new binop(binop::Implies, $1, $3); }
            | subformula OP_EQUIV subformula
	      { $$ = new binop(binop::Equiv, $1, $3); }
            | subformula OP_U subformula
	      { $$ = new binop(binop::U, $1, $3); }
            | subformula OP_R subformula
	      { $$ = new binop(binop::R, $1, $3); }
            | OP_F subformula
	      { $$ = new unop(unop::F, $2); }
            | OP_G subformula
	      { $$ = new unop(unop::G, $2); }
            | OP_X subformula
	      { $$ = new unop(unop::X, $2); }
//	    | subformula many_errors
//              { error_list->push_back(parse_error(@2, 
//		  "ignoring these unexpected trailing tokens")); 
//	        $$ = $1;
//	      }
	    
;

%%

void
yy::Parser::print_()
{
  if (looka_ == ATOMIC_PROP) 
    YYCDEBUG << " '" << *value.str << "'";
}

void
yy::Parser::error_()
{
  error_list->push_back(parse_error(location, message));
}

formula* result = 0;

namespace spot
{
  namespace ltl
  {
    parse_error_list* error_list;

    formula*
    parse(const std::string& ltl_string, 
	  parse_error_list& error_list,
	  bool debug)
    {
      result = 0;
      ltl::error_list = &error_list;
      flex_set_buffer(ltl_string.c_str());
      yy::Parser parser(debug, yy::Location());
      parser.parse();  
      return result;
    }
  }
}

// Local Variables:
// mode: c++
// End:
