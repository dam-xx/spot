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
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include "public.hh"
#include "ltlast/allnodes.hh"
#include "ltlvisit/destroy.hh"

namespace spot
{
  namespace eltl
  {
    /// The following parser allows one to define aliases of automaton
    /// operators such as: F=U(true,$0). Internally it's handled by
    /// creating a small AST associated with each alias in order to
    /// instanciate the right automatop after: U(constant(1), AP(f))
    /// for the formula F(f).
    ///
    struct alias
    {
      virtual ~alias() {};
    };
    /// We use boost::shared_ptr to easily handle deletion.
    typedef boost::shared_ptr<alias> alias_ptr;

    struct alias_not : alias
    {
      alias_ptr child;
    };
    struct alias_binop : alias
    {
      binop::type op;
      alias_ptr lhs;
      alias_ptr rhs;
    };
    struct alias_multop : alias
    {
      multop::type op;
      alias_ptr lhs;
      alias_ptr rhs;
    };
    struct alias_nfa : alias
    {
      std::vector<alias_ptr> children;
      spot::ltl::nfa::ptr nfa;
    };
    struct alias_arg : alias
    {
      int i;
    };

    typedef std::map<std::string, nfa::ptr> nfamap;
    typedef std::map<std::string, alias_ptr> aliasmap;

    /// Implementation details for error handling.
    struct parse_error_list_t
    {
      parse_error_list list_;
      std::string file_;
    };

    /// TODO: maybe implementing a visitor could be better than
    /// dynamic casting all the time here.

    /// Instanciate the formula corresponding to the given alias.
    static formula*
    alias2formula(const alias_ptr ap, spot::ltl::automatop::vec* v)
    {
      if (alias_not* a = dynamic_cast<alias_not*>(ap.get()))
	return unop::instance(unop::Not, alias2formula(a->child, v));
      if (alias_arg* a = dynamic_cast<alias_arg*>(ap.get()))
	return a->i == -1 ? constant::true_instance() :
	  a->i == -2 ? constant::false_instance() : v->at(a->i);
      if (alias_nfa* a = dynamic_cast<alias_nfa*>(ap.get()))
      {
	automatop::vec* va = new automatop::vec;
	std::vector<alias_ptr>::const_iterator i = a->children.begin();
	while (i != a->children.end())
	  va->push_back(alias2formula(*i++, v));
	return automatop::instance(a->nfa, va, false);
      }
      if (alias_binop* a = dynamic_cast<alias_binop*>(ap.get()))
	return binop::instance(a->op,
			       alias2formula(a->lhs, v),
			       alias2formula(a->rhs, v));
      if (alias_multop* a = dynamic_cast<alias_multop*>(ap.get()))
	return multop::instance(a->op,
			       alias2formula(a->lhs, v),
			       alias2formula(a->rhs, v));

      /* Unreachable code.  */
      assert(0);
    }

    /// Get the arity of a given alias.
    static size_t
    arity(const alias_ptr ap)
    {
      if (alias_not* a = dynamic_cast<alias_not*>(ap.get()))
	return arity(a->child);
      if (alias_arg* a = dynamic_cast<alias_arg*>(ap.get()))
	return std::max(a->i + 1, 0);
      if (alias_nfa* a = dynamic_cast<alias_nfa*>(ap.get()))
      {
	size_t res = 0;
	std::vector<alias_ptr>::const_iterator i = a->children.begin();
	while (i != a->children.end())
	  res = std::max(arity(*i++), res);
	return res;
      }
      if (alias_binop* a = dynamic_cast<alias_binop*>(ap.get()))
	return std::max(arity(a->lhs), arity(a->rhs));
      if (alias_multop* a = dynamic_cast<alias_multop*>(ap.get()))
	return std::max(arity(a->lhs), arity(a->rhs));

      /* Unreachable code.  */
      assert(0);
    }

    /// Alias an existing alias, as in Strong=G(F($0))->G(F($1)),
    /// where F is an alias.
    ///
    /// \param ap The original alias.
    /// \param v The arguments of the new alias.
    static alias_ptr
    realias(const alias_ptr ap, std::vector<alias_ptr> v)
    {
      if (alias_not* a = dynamic_cast<alias_not*>(ap.get()))
      {
	alias_not* res = new alias_not;
	res->child = realias(a->child, v);
	return alias_ptr(res);
      }
      if (alias_arg* a = dynamic_cast<alias_arg*>(ap.get())) // Do it.
	return a->i < 0 ? ap : v.at(a->i);
      if (alias_nfa* a = dynamic_cast<alias_nfa*>(ap.get()))
      {
	alias_nfa* res = new alias_nfa;
	std::vector<alias_ptr>::const_iterator i = a->children.begin();
	while (i != a->children.end())
	  res->children.push_back(realias(*i++, v));
	res->nfa = a->nfa;
	return alias_ptr(res);
      }
      if (alias_binop* a = dynamic_cast<alias_binop*>(ap.get()))
      {
	alias_binop* res = new alias_binop;
	res->op = a->op;
	res->lhs = realias(a->lhs, v);
	res->rhs = realias(a->rhs, v);
	return alias_ptr(res);
      }
      if (alias_multop* a = dynamic_cast<alias_multop*>(ap.get()))
      {
	alias_multop* res = new alias_multop;
	res->op = a->op;
	res->lhs = realias(a->lhs, v);
	res->rhs = realias(a->rhs, v);
	return alias_ptr(res);
      }

      /* Unreachable code.  */
      assert(0);
    }
  }
}

#define PARSE_ERROR(Loc, Msg)			\
  pe.list_.push_back				\
    (parse_error(Loc, spair(pe.file_, Msg)))

#define CHECK_EXISTING_NMAP(Loc, Ident)			\
  {							\
    nfamap::const_iterator i = nmap.find(*Ident);	\
    if (i == nmap.end())				\
    {							\
      std::string s = "unknown automaton operator `";	\
      s += *Ident;					\
      s += "'";						\
      PARSE_ERROR(Loc, s);				\
      delete Ident;					\
      YYERROR;						\
    }							\
  }

#define CHECK_ARITY(Loc, Ident, A1, A2)			\
  {							\
    if (A1 != A2)					\
    {							\
      std::ostringstream oss1;				\
      oss1 << A1;					\
      std::ostringstream oss2;				\
      oss2 << A2;					\
							\
      std::string s(*Ident);				\
      s += " is used with ";				\
      s += oss1.str();					\
      s += " arguments, but has an arity of ";		\
      s += oss2.str();					\
      PARSE_ERROR(Loc, s);				\
      delete Ident;					\
      YYERROR;						\
    }							\
  }

}

%parse-param {spot::eltl::nfamap& nmap}
%parse-param {spot::eltl::aliasmap& amap}
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

  /// To handle aliases.
  spot::eltl::alias* pval;
  spot::eltl::alias_nfa* bval;
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

%token <sval>	ATOMIC_PROP "atomic proposition"
		IDENT "identifier"

%token <ival>	ARG "argument"
		STATE "state"
		OP_OR "or operator"
		OP_XOR "xor operator"
		OP_AND "and operator"
		OP_IMPLIES "implication operator"
		OP_EQUIV "equivalent operator"
		OP_NOT "not operator"

%token		ACC "accept"
		EQ "="
		LPAREN "("
		RPAREN ")"
		COMMA ","
		END_OF_FILE "end of file"
		CONST_TRUE "constant true"
		CONST_FALSE "constant false"

/* Priorities.  */

%left OP_OR
%left OP_XOR
%left OP_AND
%left OP_IMPLIES OP_EQUIV

%left ATOMIC_PROP

%nonassoc OP_NOT

%type <nval> nfa_def
%type <fval> subformula
%type <aval> arg_list
%type <ival> nfa_arg
%type <pval> nfa_alias
%type <pval> nfa_alias_arg
%type <bval> nfa_alias_arg_list

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

nfa_list: /* empty */
        | nfa_list nfa
;

nfa: IDENT "=" "(" nfa_def ")"
	{
	  $4->set_name(*$1);
          nmap[*$1] = nfa::ptr($4);
	  delete $1;
        }
   | IDENT "=" nfa_alias
        {
	  /// Recursivity issues of aliases are handled by a parse error.
	  aliasmap::iterator i = amap.find(*$1);
	  if (i != amap.end())
	  {
	    std::string s = "`";
	    s += *$1;
	    s += "' is already aliased";
	    PARSE_ERROR(@1, s);
	    delete $1;
	    YYERROR;
	  }
	  amap[*$1] = alias_ptr($3);
   	  delete $1;
   	}
;

nfa_def: /* empty */
        {
	  $$ = new nfa;
        }
        | nfa_def STATE STATE nfa_arg
        {
	  $1->add_transition($2, $3, $4);
	  $$ = $1;
        }
        | nfa_def ACC STATE
        {
 	  $1->set_final($3);
	  $$ = $1;
        }
;

nfa_alias: IDENT "(" nfa_alias_arg_list ")"
	{
	  aliasmap::const_iterator i = amap.find(*$1);
	  if (i != amap.end())
	  {
	    CHECK_ARITY(@1, $1, $3->children.size(), arity(i->second));

	    // Hack to return the right type without screwing with the
	    // boost::shared_ptr memory handling by using get for
	    // example. FIXME: Wait for the next version of boost and
	    // modify the %union to handle alias_ptr.
	    alias_not* tmp1 = new alias_not;
	    tmp1->child = realias(i->second, $3->children);
	    alias_not* tmp2 = new alias_not;
	    tmp2->child = alias_ptr(tmp1);
	    $$ = tmp2;
	    delete $3;
	  }
	  else
	  {
	    CHECK_EXISTING_NMAP(@1, $1);
	    nfa::ptr np = nmap[*$1];

	    CHECK_ARITY(@1, $1, $3->children.size(), np->arity());
	    $3->nfa = np;
	    $$ = $3;
	  }
	  delete $1;
	}
	| OP_NOT nfa_alias
	{
	  alias_not* res = new alias_not;
	  res->child = alias_ptr($2);
	  $$ = res;
	}
	| nfa_alias OP_IMPLIES nfa_alias
	{
	  alias_binop* res = new alias_binop;
	  res->op = binop::Implies;
	  res->lhs = alias_ptr($1);
	  res->rhs = alias_ptr($3);
	  $$ = res;
	}
	// More TBD here.

nfa_alias_arg: nfa_arg
	{
	  alias_arg* res = new alias_arg;
	  res->i = $1;
	  $$ = res;
	}
        | CONST_FALSE
	{
	  alias_arg* res = new alias_arg;
	  res->i = -2;
	  $$ = res;
	}
	| OP_NOT nfa_alias_arg
	{
	  alias_not* res = new alias_not;
	  res->child = alias_ptr($2);
	  $$ = res;
	}
	// More TBD here.

nfa_alias_arg_list: nfa_alias_arg
	{
	  $$ = new alias_nfa;
	  $$->children.push_back(alias_ptr($1));
	}
	| nfa_alias // Cannot factorize because <pval> != <bval>.
	{
	  $$ = new alias_nfa;
	  $$->children.push_back(alias_ptr($1));
	}
	| nfa_alias_arg_list "," nfa_alias_arg
	{
	  $1->children.push_back(alias_ptr($3));
	  $$ = $1;
	}
;

nfa_arg: ARG
	{
	  if ($1 == -1)
	  {
	    std::string s = "out of range integer";
	    PARSE_ERROR(@1, s);
	    YYERROR;
	  }
	  $$ = $1;
	}
	| CONST_TRUE
	{ $$ = -1; }
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
	  | subformula ATOMIC_PROP subformula
	{
	  aliasmap::iterator i = amap.find(*$2);
	  if (i != amap.end())
	  {
	    CHECK_ARITY(@1, $2, 2, arity(i->second));
	    automatop::vec* v = new automatop::vec;
	    v->push_back($1);
	    v->push_back($3);
	    $$ = alias2formula(i->second, v);
	    delete v;
	  }
	  else
	  {
	    CHECK_EXISTING_NMAP(@1, $2);
	    nfa::ptr np = nmap[*$2];
	    CHECK_ARITY(@1, $2, 2, np->arity());
	    automatop::vec* v = new automatop::vec;
	    v->push_back($1);
	    v->push_back($3);
	    $$ = automatop::instance(np, v, false);
	  }
	  delete $2;
	}
	  | ATOMIC_PROP "(" arg_list ")"
	{
	  aliasmap::iterator i = amap.find(*$1);
	  if (i != amap.end())
	  {
	    CHECK_ARITY(@1, $1, $3->size(), arity(i->second));
	    $$ = alias2formula(i->second, $3);
	    delete $3;
	  }
	  else
	  {
	    CHECK_EXISTING_NMAP(@1, $1);
	    nfa::ptr np = nmap[*$1];

	    CHECK_ARITY(@1, $1, $3->size(), np->arity());
	    $$ = automatop::instance(np, $3, false);
	  }
	  delete $1;
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
      aliasmap amap;
      parse_error_list_t pe;
      pe.file_ = name;
      eltlyy::parser parser(nmap, amap, pe, env, result);
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
      aliasmap amap;
      parse_error_list_t pe;
      eltlyy::parser parser(nmap, amap, pe, env, result);
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
