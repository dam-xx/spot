%{
#include <string>
#include "public.hh"
#include "ltlast/allnodes.hh"

extern spot::ltl::formulae* result;

%}

%debug
%error-verbose
%union
{
  int token;
  std::string* str;
  spot::ltl::formulae* ltl;
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

%type <ltl> result ltl_formulae subformulae

%%
result:       ltl_formulae END_OF_INPUT
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

ltl_formulae: subformulae
              { $$ = $1; }
	    | many_errors_diagnosed subformulae
              { $$ = $2; }
	    | ltl_formulae many_errors_diagnosed
              { $$ = $1; }

many_errors: error
	    | many_errors error

subformulae: ATOMIC_PROP
	      { $$ = new atomic_prop(*$1); delete $1; }
	    | CONST_TRUE
              { $$ = new constant(constant::True); }
	    | CONST_FALSE
              { $$ = new constant(constant::False); }
	    | PAR_OPEN subformulae PAR_CLOSE
	      { $$ = $2; }
	    | PAR_OPEN error PAR_CLOSE
              { error_list->push_back(parse_error(@$, 
                 "treating this parenthetical block as false")); 
	        $$ = new constant(constant::False);
	      }
	    | PAR_OPEN subformulae many_errors PAR_CLOSE
              { error_list->push_back(parse_error(@3, 
				      "unexpected input ignored")); 
	        $$ = $2;
	      }
	    | OP_NOT subformulae
	      { $$ = new unop(unop::Not, $2); }
            | subformulae OP_AND subformulae
	      { $$ = new multop(multop::And, $1, $3); }
	    | subformulae OP_OR subformulae
	      { $$ = new multop(multop::Or, $1, $3); }
	    | subformulae OP_XOR subformulae
	      { $$ = new binop(binop::Xor, $1, $3); }
	    | subformulae OP_IMPLIES subformulae
	      { $$ = new binop(binop::Implies, $1, $3); }
            | subformulae OP_EQUIV subformulae
	      { $$ = new binop(binop::Equiv, $1, $3); }
            | subformulae OP_U subformulae
	      { $$ = new binop(binop::U, $1, $3); }
            | subformulae OP_R subformulae
	      { $$ = new binop(binop::R, $1, $3); }
            | OP_F subformulae
	      { $$ = new unop(unop::F, $2); }
            | OP_G subformulae
	      { $$ = new unop(unop::G, $2); }
            | OP_X subformulae
	      { $$ = new unop(unop::X, $2); }
//	    | subformulae many_errors
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

formulae* result = 0;

namespace spot
{
  namespace ltl
  {
    parse_error_list* error_list;

    formulae*
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
