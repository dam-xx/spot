/* Copyright (C) 2008 Laboratoire d'Informatique de
** Paris 6 (LIP6), département Systèmes Répartis Coopératifs (SRC),
** Université Pierre et Marie Curie.
**
** This file is part of Spot, a model checking library.
**
** Spot is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Spot is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
** or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
** License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Spot; see the file COPYING.  If not, write to the Free
** Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
** 02111-1307, USA.
*/
%language "C++"
%locations
%defines
%name-prefix "eltlyy"
%debug
%error-verbose

%code requires
{
#include <string>
#include <sstream>
#include <limits>
#include <cerrno>
#include "public.hh"
#include "ltlast/allnodes.hh"
#include "ltlvisit/destroy.hh"

// Implementation details for error handling.
namespace spot
{
  namespace eltl
  {
    struct parse_error_list_t
    {
      parse_error_list list_;
      std::string file_;
    };
  }
}

#define PARSE_ERROR(Loc, Msg) \
  pe.list_.push_back \
    (parse_error(Loc, spair(pe.file_, Msg)))
}

%parse-param {spot::eltl::nfamap& nmap}
%parse-param {spot::eltl::parse_error_list_t &pe}
%parse-param {spot::ltl::environment &parse_environment}
%parse-param {spot::ltl::formula* &result}
%lex-param {spot::eltl::parse_error_list_t &pe}
%expect 0
%pure-parser
%union
{
  int ival;
  std::string* sval;
  spot::ltl::nfa* nval;
  spot::ltl::automatop::vec* aval;
  spot::ltl::formula* fval;
}

%code {
/* ltlparse.hh and parsedecl.hh include each other recursively.
   We mut ensure that YYSTYPE is declared (by the above %union)
   before parsedecl.hh uses it. */
#include "parsedecl.hh"
using namespace spot::eltl;
using namespace spot::ltl;
}

/* All tokens.  */

%token <sval> ATOMIC_PROP "atomic proposition"
	      IDENT "identifier"

%token <ival> ARG "argument"
	      STATE "state"
	      OP_OR "or operator"
	      OP_XOR "xor operator"
	      OP_AND "and operator"
	      OP_IMPLIES "implication operator"
	      OP_EQUIV "equivalent operator"
	      OP_NOT "not operator"

%token ACC "accept"
       EQ "="
       LPAREN "("
       RPAREN ")"
       COMMA ","
       END_OF_FILE "end of file"
       CONST_TRUE "constant true"
       CONST_FALSE "constant false"

/* Priorities.  */

/* Logical operators.  */
%left OP_OR
%left OP_XOR
%left OP_AND
%left OP_IMPLIES OP_EQUIV

%nonassoc OP_NOT

%type <nval> nfa_def
%type <fval> subformula
%type <aval> arg_list

%destructor { delete $$; } "atomic proposition"
%destructor { spot::ltl::destroy($$); } subformula

%printer { debug_stream() << *$$; } "atomic proposition"

%%

result: nfa_list subformula
	{
	  result = $2;
	  YYACCEPT;
	}
;

/* NFA definitions. */

nfa_list: /* empty. */
        | nfa_list nfa
;

nfa: IDENT "=" "(" nfa_def ")"
	{
	  $4->set_name(*$1);
          nmap[*$1] = nfa::ptr($4);
	  delete $1;
        }
;

nfa_def: /* empty. */
        {
	  $$ = new nfa();
        }
        | nfa_def STATE STATE ARG
        {
	  if ($4 == -1 || $3 == -1 || $2 == -1)
	  {
	    std::string s = "out of range integer";
	    PARSE_ERROR(@1, s);
	    YYERROR;
	  }
	  $1->add_transition($2, $3, $4);
	  $$ = $1;
        }
        | nfa_def STATE STATE CONST_TRUE
        {
	  $1->add_transition($2, $3, -1);
	  $$ = $1;
	}
        | nfa_def ACC STATE
        {
 	  $1->set_final($3);
	  $$ = $1;
        }
;

/* Formulae. */

subformula: ATOMIC_PROP
	{
	   $$ = parse_environment.require(*$1);
	   if (!$$)
	   {
	     std::string s = "unknown atomic proposition `";
	     s += *$1;
	     s += "' in environment `";
	     s += parse_environment.name();
	     s += "'";
	     PARSE_ERROR(@1, s);
	     delete $1;
	     YYERROR;
	   }
	   else
	     delete $1;
	}
	  | ATOMIC_PROP "(" arg_list ")"
	{
	  nfamap::iterator i = nmap.find(*$1);
	  if (i == nmap.end())
	  {
	    std::string s = "unknown automaton operator `";
	    s += *$1;
	    s += "'";
	    PARSE_ERROR(@1, s);
	    delete $1;
	    YYERROR;
	  }

	  automatop* res = automatop::instance(i->second, $3);
	  if (res->size() != i->second->arity())
	  {
	    std::ostringstream oss1;
	    oss1 << res->size();
	    std::ostringstream oss2;
	    oss2 << i->second->arity();

	    std::string s(*$1);
	    s += " is used with ";
	    s += oss1.str();
	    s += " arguments, but has an arity of ";
	    s += oss2.str();
	    PARSE_ERROR(@1, s);
	    delete $1;
	    delete $3;
	    YYERROR;
	  }
	  delete $1;
	  $$ = res;
	}
	  | CONST_TRUE
	{ $$ = constant::true_instance(); }
	  | CONST_FALSE
	{ $$ = constant::false_instance(); }
          | "(" subformula ")"
	{ $$ = $2; }
          | subformula OP_AND subformula
	{ $$ = multop::instance(multop::And, $1, $3); }
          | subformula OP_OR subformula
	{ $$ = multop::instance(multop::Or, $1, $3); }
	  | subformula OP_XOR subformula
	{ $$ = binop::instance(binop::Xor, $1, $3); }
	  | subformula OP_IMPLIES subformula
        { $$ = binop::instance(binop::Implies, $1, $3); }
	  | subformula OP_EQUIV subformula
	{ $$ = binop::instance(binop::Equiv, $1, $3); }
          | OP_NOT subformula
	{ $$ = unop::instance(unop::Not, $2); }
;

arg_list: subformula
	{
	  // TODO: delete it whenever a parse error occurs on a subformula.
	  $$ = new automatop::vec;
	  $$->push_back($1);
	}
	| arg_list "," subformula
	{
	  $1->push_back($3);
	  $$ = $1;
	}
;

%%

void
eltlyy::parser::error(const location_type& loc, const std::string& s)
{
  PARSE_ERROR(loc, s);
}

namespace spot
{
  namespace eltl
  {
    formula*
    parse_file(const std::string& name,
	       parse_error_list& error_list,
	       environment& env,
	       bool debug)
    {
      if (flex_open(name))
      {
	error_list.push_back
	  (parse_error(eltlyy::location(),
		       spair("-", std::string("Cannot open file ") + name)));
	return 0;
      }
      formula* result = 0;
      nfamap nmap;
      parse_error_list_t pe;
      pe.file_ = name;
      eltlyy::parser parser(nmap, pe, env, result);
      parser.set_debug_level(debug);
      parser.parse();
      error_list = pe.list_;
      flex_close();
      return result;
    }

    formula*
    parse_string(const std::string& eltl_string,
		 parse_error_list& error_list,
		 environment& env,
		 bool debug)
    {
      flex_scan_string(eltl_string.c_str());
      formula* result = 0;
      nfamap nmap;
      parse_error_list_t pe;
      eltlyy::parser parser(nmap, pe, env, result);
      parser.set_debug_level(debug);
      parser.parse();
      error_list = pe.list_;
      return result;
    }
  }
}

// Local Variables:
// mode: c++
// End:
