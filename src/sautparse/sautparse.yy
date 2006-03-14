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
#include <string>
#include <sstream>
#include <list>
#include "public.hh"
#include "saut/saut.hh"
#include "saut/sync.hh"
#include "tgbaalgos/dotty.hh"
#include "ltlparse/public.hh"
#include "ltlvisit/destroy.hh"

namespace
{
  typedef std::map<std::string, spot::saut*> aut_map;
  typedef std::map<const spot::saut*, std::string> aut_names;
  typedef std::map<std::string, spot::sync*> sync_map;
  struct context_t
  {
    spot::saut* aut;
    spot::sync* syn;
    aut_map auts;
    aut_names names;
    sync_map syns;
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
%token DASH "-"
%token ARROW "->"
%token SEMICOLON ";"
%token COMA ","
%token DOT "."
%token VERIFIES "|="
%token <str> QSTRING "quoted string"
%type <idlist> idtuple idepstuple ap_states
%type <str> ideps ap_state
%type <autlist> auttuple
%type <prop> ap_prop ap_props
%type <f> ltlformula

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
	       spot::saut*& a = context.auts[*$1];
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
	       spot::sync*& a = context.syns[*$1];
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
	{ context.aut->declare_propositions($1, *$3); delete $3; }

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
	      *$$ = bdd_ithvar(p);
	    }
	}

tabledefs: tabledefsheader ";" tabledefsbody
	| tabledefsheader ";" tabledefsbody ";" tabledefsoptions

tabledefsbody: tabledefbody
	| tabledefsbody "," tabledefbody
	;

tabledefsheader: "(" auttuple ")"
	{
	  context.syn = new spot::sync(*$2);
	  delete $2;
        }

auttuple: IDENT
	{
	  aut_map::const_iterator it = context.auts.find(*$1);
	  if (it == context.auts.end())
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
	  aut_map::const_iterator it = context.auts.find(*$3);
	  if (it == context.auts.end())
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

tabledefbody: "(" idepstuple ")"
	{
	  if ($2->size() != context.syn->size())
	    {
	      assert($2->size() < context.syn->size());
              error_list.push_back(spot::saut_parse_error(@$,
                                   "not enough actions or too many automata"));
            }
          else if (context.syn->declare_rule(*$2))
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

command: "Check" "(" IDENT "," ltlformula "," QSTRING ")"
        | "Display" "(" IDENT "," ltlformula ")"
	{
	  if (error_list.empty() || !$5)
	    {
	      sync_map::const_iterator i = context.syns.find(*$3);
	      if (i == context.syns.end())
		{
		  error_list.push_back(spot::saut_parse_error(@3, *$3 +
				     ": unknown table"));
		}
	      else
		{
                   i->second->set_aphi($5);
		   spot::ltl::destroy($5);
		   spot::dotty_reachable(std::cout, i->second);
		}
            }
	  else
	    {
               error_list.push_back(spot::saut_parse_error(@$,
	                            "ignored due to previous errors"));
	    }
        }
	| "Display" "(" IDENT ")"
	{
	  if (error_list.empty())
	    {
	      sync_map::const_iterator i = context.syns.find(*$3);
	      if (i == context.syns.end())
		{
		  error_list.push_back(spot::saut_parse_error(@3, *$3 +
				     ": unknown table"));
		}
	      else
		{
		  spot::dotty_reachable(std::cout, i->second);
		}
            }
	  else
	    {
               error_list.push_back(spot::saut_parse_error(@$,
	                            "ignored due to previous errors"));
	    }
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
	      here.begin.column += j->first.begin.column;
	      here.end.line =
		here.begin.line + j->first.end.line - j->first.begin.line;
	      here.end.column =
		here.begin.column + j->first.end.column - j->first.begin.column;
	      error_list.push_back(spot::saut_parse_error(here,
							  j->second));
	    }
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

  saut*
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
    context_t context;
    context.env = &env;
    sautyy::parser parser(error_list, context, d);
    parser.set_debug_level(debug);
    parser.parse();
    sautyyclose();
    return 0;
  }
}
