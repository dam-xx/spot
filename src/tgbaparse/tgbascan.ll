%option noyywrap
%option prefix="tgbayy"
%option outfile="lex.yy.c"

%{
#include <string>
#include "parsedecl.hh"

#define YY_USER_ACTION \
  yylloc->columns(yyleng);

#define YY_USER_INIT                            \
  do {                                          \
    yylloc->begin.filename = current_file;      \
    yylloc->end.filename = current_file;        \
  } while (0)

#define YY_NEVER_INTERACTIVE 1

static std::string current_file;

%}

eol      \n|\r|\n\r|\r\n

%%

%{
  yylloc->step ();
%}

\"[^\"]*\"		{ 
			  yylval->str = new std::string(yytext + 1, 
			                                yyleng - 2);
	                  return STRING;
		        }

acc[ \t]*=		return ACC_DEF;

[a-zA-Z][a-zA-Z0-9_]*   {
			  yylval->str = new std::string(yytext);
	                  return IDENT;
		        }

			/* discard whitespace */ 
{eol}			yylloc->lines(yyleng); yylloc->step();
[ \t]+			yylloc->step(); 

.			return *yytext;

%{
  /* Dummy use of yyunput to shut up a gcc warning.  */
  (void) &yyunput;
%}

%%

namespace spot
{
  int
  tgbayyopen(const std::string &name)
  {
    if (name == "-")
      {
        yyin = stdin;
        current_file = "standard input";
      }
    else
      {
        yyin = fopen (name.c_str (), "r");
        current_file = name;
        if (!yyin)
	  return 1;
      }
    return 0;
  }

  void
  tgbayyclose()
  {
    fclose(yyin);
  }
}