#ifndef SPOT_LTLPARSE_PARSEDECL_HH
# define SPOT_LTLPARSE_PARSEDECL_HH

#include "ltlparse.hh"
#include "location.hh"

# define YY_DECL \
  int yylex (yystype *yylval, yy::Location *yylloc)
YY_DECL;

void flex_set_buffer(const char *buf);

#endif // SPOT_LTLPARSE_PARSEDECL_HH

