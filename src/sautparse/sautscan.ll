/* Copyright (C) 2006  Laboratoire d'Informatique de Paris 6 (LIP6),
** d�partement Syst�mes R�partis Coop�ratifs (SRC), Universit� Pierre
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
%option noyywrap
%option prefix="sautscan"
%option outfile="lex.yy.c"
%x STATE_STRING

%{
#include <string>
#include "sautparse.hh"

#define YY_USER_ACTION \
  yylloc->columns(yyleng);

#define YY_NEVER_INTERACTIVE 1

%}

eol      \n|\r|\n\r|\r\n
ws	 " "|\t

%%

%{
  yylloc->step ();
%}

"("				return LPAREN;
")"				return RPAREN;
"-"				return DASH;
"->"				return ARROW;
";"				return SEMICOLON;
","				return COMA;
"."				return DOT;

:={ws}*Automaton		return AUTOMATON;
:={ws}*Table			return TABLE;

Nodes				return NODES;
Transitions			return TRANSITIONS;
AtomicPropositions		return ATOMICPROPOSITIONS;
Check				return CHECK;

[A-Za-z_][A-Za-z0-9_?!.:]*	{
				  yylval->str = new std::string(yytext, yyleng);
				  return IDENT;
				}

{eol}				yylloc->lines(yyleng); yylloc->step();
{ws}+				yylloc->step();
.				return *yytext;

%{
  /* Dummy use of yyunput to shut up a gcc warning.  */
  (void) &yyunput;
%}

%%

namespace spot
{
  int
  sautyyopen(const std::string &name)
  {
    if (name == "-")
      {
        yyin = stdin;
      }
    else
      {
        yyin = fopen(name.c_str(), "r");
        if (!yyin)
          return 1;
      }
    return 0;
  }

  void
  sautyyclose()
  {
    fclose(yyin);
  }
}