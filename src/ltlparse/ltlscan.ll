%option noyywrap

%{
#include <string>
#include "parsedecl.hh"

/* Hack Flex so we read from a string instead of reading from a file.  */
# define YY_INPUT(buf, result, max_size)				\
  do {									\
    result = (max_size < to_parse_size) ? max_size : to_parse_size;	\
    memcpy(buf, to_parse, result);					\
    to_parse_size -= result;						\
    to_parse += result;							\
  } while (0);

#define YY_USER_ACTION \
    yylloc->columns (yyleng);

static const char *to_parse = 0;
static size_t to_parse_size = 0;
  
void
flex_set_buffer(const char *buf)
{
  to_parse = buf;
  to_parse_size = strlen(to_parse);
}

%}

%%

%{
  yylloc->step ();
%}

"("			return PAR_OPEN;
")"			return PAR_CLOSE;

"!"			return OP_NOT;
  /* & and | come from Spin.  && and || from LTL2BA.  */
"||"|"|"|"+"		return OP_OR;
"&&"|"&"|"."|"*"	return OP_AND;
"^"			return OP_XOR;
"=>"|"->"		return OP_IMPLIES;
"<=>"|"<->"		return OP_EQUIV;

  /* <>, [], and () are used in Spin.  */
"F"|"<>"		return OP_F;
"G"|"[]"		return OP_G;
"U"			return OP_U;
"R"|"V"			return OP_R;
"X"|"()"		return OP_X;

"1"|"true"		return CONST_TRUE;
"0"|"false"		return CONST_FALSE;

[ \t\n]+		/* discard whitespace */ yylloc->step (); 

  /* An Atomic proposition cannot start with the letter
     used by a unary operator (F,G,X), unless this
     letter is followed by a digit in which case we assume
     it's an ATOMIC_PROP (even though F0 could be seen as Ffalse).  */
[a-zA-EH-WYZ_][a-zA-Z0-9_]* | 
[FGX][0-9_][a-zA-Z0-9_]* { 
		  yylval->str = new std::string(yytext); 
	          return ATOMIC_PROP;
		}

.		return *yytext;

<<EOF>>		return END_OF_INPUT;

%{
  /* Dummy use of yyunput to shut up a gcc warning.  */
  (void) &yyunput;
%}
