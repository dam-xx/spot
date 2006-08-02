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
%option noyywrap
%option prefix="sautscan"
%option outfile="lex.yy.c"
%x STATE_STRING

%{
#include <string>
#include "sautparse/parsedecl.hh"

#define YY_USER_ACTION \
  yylloc->columns(yyleng);

#define YY_NEVER_INTERACTIVE 1

typedef sautyy::parser::token token;
%}

eol      \n|\r|\n\r|\r\n
ws	 " "|\t

%%

%{
  yylloc->step ();
%}

"("				return token::LPAREN;
")"				return token::RPAREN;
"-"				return token::DASH;
"->"				return token::ARROW;
";"				return token::SEMICOLON;
","				return token::COMA;
"."				return token::DOT;
"|="				return token::VERIFIES;

:={ws}*Automaton		return token::AUTOMATON;
:={ws}*Table			return token::TABLE;

:{ws}*Weak			return token::WEAK;
:{ws}*Strong			return token::STRONG;

Nodes				return token::NODES;
Transitions			return token::TRANSITIONS;
AtomicPropositions		return token::ATOMICPROPOSITIONS;
Check				return token::CHECK;
Display				return token::DISPLAY;

[A-Za-z0-9_?!.:]+		{
				  yylval->str = new std::string(yytext, yyleng);
				  return token::IDENT;
				}

(#.*)?{eol}			yylloc->lines(); yylloc->step();
{ws}+				yylloc->step();


\"				{
				  yylval->str = new std::string;
				  BEGIN(STATE_STRING);
				}

.				return *yytext;

  /* Handle \" and \\ in strings.  */
<STATE_STRING>{
  \"                    	{
                        	  BEGIN(INITIAL);
				  return token::QSTRING;
                        	}
  \\["\\]               	yylval->str->append(1, yytext[1]);
  [^"\\]+               	yylval->str->append(yytext, yyleng);
}


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
