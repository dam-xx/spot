/* Copyright (C) 2006  Laboratoire d'Informatique de Paris 6 (LIP6),
** département Systèmes Répartis Coopératifs (SRC), Université Pierre
** et Marie Curie.
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

%{
#include <sstream>
#include <map>
#include <string>
#include "public.hh"
#include "saut/saut.hh"
#include "saut/sync.hh"
#include "tgbaalgos/dotty.hh"
#include "ltlparse/public.hh"
#include "ltlvisit/destroy.hh"
#include "tgbaalgos/emptiness.hh"
#include "tgbaalgos/ltl2tgba_fm.hh"
#include "tgba/tgbaproduct.hh"

namespace
{
  typedef std::map<const spot::saut*, std::string> aut_names;
  struct context_t
  {
    spot::saut_parse_result* res;
    spot::saut* aut;
    spot::sync* syn;
    aut_names names;
    spot::ltl::environment* env;
  };

  void free_idlist(std::list<const std::string*>*);
}

%}


%parse-param {spot::saut_parse_error_list& error_list}
%parse-param {context_t& context}
%parse-param {spot::bdd_dict* dict}
%debug
%error-verbose
%union
{
  int token;
  std::string* str;
  std::list<const std::string*>* idlist;
  spot::sync::saut_list* autlist;
  bdd* prop;
  spot::ltl::formula* f;
  spot::sync* table;
  spot::emptiness_check_instantiator* ec;
  spot::sync::action_vect* actvec;
}

%{
/* sautparse.hh and parsedecl.hh include each other recursively.
   We mut ensure that YYSTYPE is declared (by the above %union)
   before parsedecl.hh uses it. */
#include "parsedecl.hh"
using namespace spot::ltl;

/* Ugly hack so that Bison use sautyylex, not yylex.
   (%name-prefix doesn't work for the lalr1.cc skeleton
   at the time of writing.)  */
#define yylex sautyylex
%}

%token <str> IDENT "identifier"
%token AUTOMATON "=Automaton"
%token TABLE "=Table"
%token LPAREN "("
%token RPAREN ")"
%token NODES "Nodes"
%token TRANSITIONS "Transitions"
%token ATOMICPROPOSITIONS "AtomicPropositions"
%token CHECK "Check"
%token DISPLAY "Display"
%token WEAK ":Weak"
%token STRONG ":Strong"
%token DASH "-"
%token ARROW "->"
%token SEMICOLON ";"
%token COMA ","
%token DOT "."
%token VERIFIES "|="
%token <str> QSTRING "quoted string"
%type <idlist> idtuple idepstuple ap_states
%type <str> ideps ap_state IDENT_or_QSTRING
%type <autlist> auttuple
%type <prop> ap_prop ap_props
%type <f> ltlformula
%type <table> tableid
%type <ec> emptinesscheck
%type <actvec> tabledefbody

%destructor { delete $$; } IDENT QSTRING ideps ap_state
                           IDENT_or_QSTRING emptinesscheck
%destructor { spot::ltl::destroy($$); } ltlformula
%destructor { free_idlist($$); } idtuple idepstuple ap_states
%destructor { for (spot::sync::saut_list::iterator i = $$->begin();
		   i != $$->end(); ++i)
                     delete *i;
             } auttuple
%%

system: definitions commands

definitions: definition
	| definitions definition
	;

commands: command
	| commands command
	;

definition: assignment

assignment: IDENT structure
        {
	  if (context.aut)
            {
	       spot::saut*& a = context.res->auts[*$1];
	       context.names[context.aut] = *$1;
	       delete $1;
	       if (a)
	         delete a;
	       context.aut->finish();
	       a = context.aut;
	       context.aut = 0;
            }
	  else if (context.syn)
            {
	       spot::sync*& a = context.res->syns[*$1];
	       delete $1;
	       if (a)
	         delete a;
	       a = context.syn;
	       context.syn = 0;
            }
        }


structure: "=Automaton" "(" { context.aut = new spot::saut(dict); } automatondefs ")"
	| "=Table" "(" tabledefs ")"
	;

automatondefs: automatondef
	| automatondefs automatondef
	;

automatondef: "Nodes" "(" idtuple ")"
	{ context.aut->declare_nodes($3); free_idlist($3); }
	| "Transitions" "(" transitions ")"
	| "AtomicPropositions" "(" atomicpropositions ")"
	;

transitions: transition
	| transitions "," transition
	;

transition: IDENT "-" IDENT "->" IDENT
	{
	  context.aut->declare_transition(*$1, *$3, *$5);
	  delete $1; delete $3; delete $5;
        }

atomicpropositions: ap_statement
	| atomicpropositions "," ap_statement
	;

ap_statement: ap_states "|=" ap_props
        {
	  context.aut->declare_propositions($1, *$3);
          free_idlist($1);
          delete $3;
	}

ap_states: ap_state
	{ $$ = new std::list<const std::string*>; $$->push_back($1); }
	| ap_states ap_state
	{ $$->push_back($2); }
	;

ap_state: IDENT
	{
	   if (!context.aut->known_node(*$1))
	      error_list.push_back(spot::saut_parse_error(@1,
                                   *$1 + ": unknown node"));
        }

ap_props: ap_prop
	{ $$ = $1; }
	| ap_props ap_prop
	{ *$1 &= *$2; delete $2; }
	;

ap_prop: IDENT
        {
	  $$ = new bdd;
	  spot::ltl::formula* f = context.env->require(*$1);
	  if (!f)
	    {
	      std::string s = "acceptance condition `";
	      s += *$1;
	      s += "' unknown in environment `";
	      s += context.env->name();
	      s += "'";
	      error_list.push_back(spot::saut_parse_error(@1, s));
	      *$$ = bddfalse;
	    }
	  else
	    {
	      int p =
	        context.aut->get_dict()->register_proposition(f, context.aut);
	      spot::ltl::destroy(f);
	      *$$ = bdd_ithvar(p);
	    }
	  delete $1;
	}

tabledefs: tabledefsheader ";" tabledefsbody
	| tabledefsheader ";" tabledefsbody ";" tabledefsoptions

tabledefsbody: tabledefbodyopt
	| tabledefsbody "," tabledefbodyopt
	;

tabledefsheader: "(" auttuple ")"
	{
	  context.syn = new spot::sync(*$2);
	  delete $2;
        }

auttuple: IDENT
	{
	  spot::saut_parse_result::aut_map::const_iterator it
	    = context.res->auts.find(*$1);
	  if (it == context.res->auts.end())
	    {
               error_list.push_back(spot::saut_parse_error(@1,
	                            *$1 + ": unknown automaton"));
               YYABORT;
            }
	  delete $1;
          $$ = new spot::sync::saut_list;
          $$->push_back(it->second);
        }
	| auttuple "," IDENT
        {
	  spot::saut_parse_result::aut_map::const_iterator it
	    = context.res->auts.find(*$3);
	  if (it == context.res->auts.end())
	    {
               error_list.push_back(spot::saut_parse_error(@3,
	                            *$3 + ": unknown automaton"));
               YYABORT;
            }
          delete $3;
          $$ = $1;
          $$->push_back(it->second);
        }
	;

idtuple: IDENT
	{ $$ = new std::list<const std::string*>; $$->push_back($1); }
	| idtuple "," IDENT
	{ $$ = $1; $$->push_back($3); }
	;

tabledefbodyopt: tabledefbody
        | tabledefbody WEAK
	{ context.syn->set_fairness($1, spot::sync::action_vect::Weak); }
        | tabledefbody STRONG
	{ context.syn->set_fairness($1, spot::sync::action_vect::Strong); }
        ;

tabledefbody: "(" idepstuple ")"
	{
	  $$ = 0;
	  if ($2->size() != context.syn->size())
	    {
	      assert($2->size() < context.syn->size());
              error_list.push_back(spot::saut_parse_error(@$,
                                   "not enough actions or too many automata"));
            }
          else if (!($$ = context.syn->declare_rule(*$2)))
	    {
              error_list.push_back(spot::saut_parse_error(@2,
                                   "invalid tuple"));
            }
          free_idlist($2);
        }

tabledefsoptions :
	| tabledefsoptions IDENT
	{
	  if (*$2 == "stubborn")
	    {
	      context.syn->set_stubborn();
            }
	  else
            {
              error_list.push_back(spot::saut_parse_error(@2,
                                   *$2 + ": unknown table option"));
	    }
	  delete $2;
        }


idepstuple: ideps
	{ $$ = new std::list<const std::string*>;
	  if ($1 && !context.syn->known_action(0, *$1))
            {
              error_list.push_back(spot::saut_parse_error(@1,
                                   *$1 + ": unknown action in automaton `"
				   + context.names[context.syn->aut(0)] + "'"));
              delete $1;
  	      $$->push_back(0);
            }
	  else
	    {
  	      $$->push_back($1);
	    }
        }
	| idepstuple "," ideps
	{ $$ = $1;
          unsigned n = $$->size();
	  if (n >= context.syn->size())
	    {
	      error_list.push_back(spot::saut_parse_error(@3, *$3 +
                                 ": too many actions or not enough automata"));
            }
	  else if ($3 && !context.syn->known_action(n, *$3))
            {
	      std::ostringstream o;
	      o << *$3 << ": unknown action for automaton `"
	        << context.names[context.syn->aut(n)] << "'";
              error_list.push_back(spot::saut_parse_error(@3, o.str()));
              delete $3;
  	      $$->push_back(0);
            }
	  else
	    {
  	      $$->push_back($3);
	    }
        }
	;

ideps: IDENT    { $$ = $1; }
	| "."   { $$ = 0; }
	;

command: "Check" "(" tableid "," ltlformula "," emptinesscheck ")"
	{
	  if (error_list.empty() && $3 && $5 && $7)
	    {
	      std::string err = $3->set_aphi($5);
	      if (!err.empty())
	        {
                  error_list.push_back(spot::saut_parse_error(@5,
		       std::string("unknown atomic proposition(s) in system: ")
		       + err));
		  goto check_err;
                }
	      spot::tgba_explicit* f =
		spot::ltl_to_tgba_fm($5, dict, true, true,
				     false, false, 0,
				     spot::ltl::Reduce_All);
	      spot::tgba_product* p = new spot::tgba_product($3, f);
	      spot::emptiness_check* ec = $7->instantiate(p);
	      spot::emptiness_check_result* res = ec->check();

	      const spot::ec_statistics* ecs =
		dynamic_cast<const spot::ec_statistics*>(ec);
	      if (ecs)
		std::cout << ecs->states() << " unique states visited, "
			  << ecs->transitions() << " transitions visited, max "
			  << ecs->max_depth() << " items in stack."
			  << std::endl << std::endl;

	      if (!res)
		{
		  std::cout << "No accepting run found." << std::endl;
		}
	      else
		{
		  std::cout << "An accepting run exists." << std::endl;

		  // Disable stubborn sets when computing counterexamples.
		  bool save_stubborn = $3->get_stubborn();
		  $3->set_stubborn(false);
		  spot::tgba_run* cex = res->accepting_run();
		  if (cex)
		    {
		      std::cout << std::endl;
		      spot::print_tgba_run(std::cout, p, cex);
		      delete cex;
		    }
		  $3->set_stubborn(save_stubborn);
		}
	      delete res;
	      delete ec;
	      delete p;
	      delete f;
	    }
	  else
	    {
	    check_err:
              error_list.push_back(spot::saut_parse_error(@$,
	                           "ignored due to previous errors"));
	    }
	  if ($5)
	    spot::ltl::destroy($5);
	  if ($7)
	    delete $7;
        }
        | "Display" "(" tableid "," ltlformula ")"
	{
	  if (error_list.empty() && $3 && $5)
	    {
	      std::string err = $3->set_aphi($5);
	      if (!err.empty())
                 error_list.push_back(spot::saut_parse_error(@5,
		   std::string("unknown atomic proposition(s) in system: ")
		   + err));
	      spot::ltl::destroy($5);
	      if (!err.empty())
	        goto display_err;
	      spot::dotty_reachable(std::cout, $3);
            }
	  else
	    {
	    display_err:
	      error_list.push_back(spot::saut_parse_error(@$,
	                           "ignored due to previous errors"));
	    }
        }
	| "Display" "(" tableid ")"
	{
	  if (error_list.empty() && $3)
	    {
	      spot::dotty_reachable(std::cout, $3);
            }
	  else
	    {
	      error_list.push_back(spot::saut_parse_error(@$,
	                           "ignored due to previous errors"));
	    }
	}

emptinesscheck: IDENT_or_QSTRING
	{
	  const char* err = 0;
	  $$ = spot::emptiness_check_instantiator::construct($1->c_str(), &err);
	  if (!$$)
	    {
	      if (err == $1->c_str())
		error_list.push_back(spot::saut_parse_error(@$,
                                     "unkown emptiness check algorithm"));
	      else
		error_list.push_back(spot::saut_parse_error(@$,
		  std::string("failed to parse emptiness check option near `")
                  + err + "'"));
	    }
	  delete $1;
 	}

IDENT_or_QSTRING: IDENT | QSTRING

tableid: IDENT
	{
           spot::saut_parse_result::sync_map::const_iterator i
	     = context.res->syns.find(*$1);
	   if (i == context.res->syns.end())
	     {
	        error_list.push_back(spot::saut_parse_error(@1, *$1 +
                                     ": unknown table"));
                $$ = 0;
	     }
	   else
	     {
	       $$ = i->second;
             }
	  delete $1;
	}

ltlformula: QSTRING
	{
	  spot::ltl::parse_error_list pel;
	  $$ = spot::ltl::parse(*$1, pel, *context.env);
	  for (parse_error_list::iterator j = pel.begin();
	       j != pel.end(); ++j)
	    {
	      // Adjust the diagnostic to the current position.
	      location here = @1;
	      here.begin.line += j->first.begin.line;
	      here.begin.column += 1 + j->first.begin.column;
	      here.end.line =
		here.begin.line + j->first.end.line - j->first.begin.line;
	      here.end.column =
		here.begin.column + j->first.end.column - j->first.begin.column;
	      error_list.push_back(spot::saut_parse_error(here,
							  j->second));
	    }
	  delete $1;
	}

%%

void
yy::parser::error(const location_type& location, const std::string& message)
{
  error_list.push_back(spot::saut_parse_error(location, message));
}

namespace
{
   void
   free_idlist(std::list<const std::string*>* idlist)
   {
     for (std::list<const std::string*>::iterator i = idlist->begin();
          i != idlist->end(); ++i)
       delete *i;
     delete idlist;
   }
}

namespace spot
{

  saut_parse_result*
  saut_parse(const std::string& name,
             saut_parse_error_list& error_list,
             bdd_dict* d,
             environment& env,
             bool debug)
  {
    if (sautyyopen(name))
      {
        error_list.push_back
          (saut_parse_error(yy::location(),
                            std::string("Cannot open file ") + name));
        return 0;
      }
    saut_parse_result* res = new saut_parse_result;
    context_t context;
    context.env = &env;
    context.res = res;
    sautyy::parser parser(error_list, context, d);
    parser.set_debug_level(debug);
    parser.parse();
    sautyyclose();
    return res;
  }
}
