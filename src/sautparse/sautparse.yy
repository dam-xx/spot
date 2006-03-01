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
#include "public.hh"
%}


%parse-param {spot::saut_parse_error_list& error_list}
%debug
%error-verbose
%union
{
  int token;
  std::string* str;
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
%token DASH "-"
%token ARROW "->"
%token SEMICOLON ";"
%token COMA ","
%token DOT "."

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

structure: "=Automaton" "(" automatondefs ")"
	| "=Table" "(" tabledefs ")"
	;

automatondefs: automatondef
	| automatondefs automatondef
	;

automatondef: "Nodes" "(" idtuple ")"
	| "Transitions" "(" transitions ")"
	| "AtomicPropositions" "(" atomicpropositions ")"
	;

transitions: transition
	| transitions "," transition
	;

transition: IDENT "-" IDENT "->" IDENT

atomicpropositions: ap_statement
	| atomicpropositions "," ap_statement
	;

ap_statement: ap_states "|=" ap_props

ap_states: IDENT
	| ap_states IDENT
	;

ap_props: IDENT
	| ap_props IDENT
	;

tabledefs: tabledefsheader ";" tabledefsbody

tabledefsbody: tabledefbody
	| tabledefsbody "," tabledefbody
	;

tabledefsheader: "(" idtuple ")"

idtuple: IDENT
	| idtuple "," IDENT
	;

tabledefbody: "(" idepstuple ")"

idepstuple: ideps
	| idepstuple "," ideps
	;

ideps: IDENT
	| "."
	;

command: "Check" "(" IDENT ")"

%%

void
yy::parser::error(const location_type& location, const std::string& message)
{
  error_list.push_back(spot::saut_parse_error(location, message));
}

namespace spot
{
  saut*
  saut_parse(const std::string& name,
             saut_parse_error_list& error_list,
             bdd_dict*,
             environment&,
             bool debug)
  {
    if (sautyyopen(name))
      {
        error_list.push_back
          (saut_parse_error(yy::location(),
                            std::string("Cannot open file ") + name));
        return 0;
      }
    sautyy::parser parser(error_list);
    parser.set_debug_level(debug);
    parser.parse();
    sautyyclose();
    return 0;
  }
}
