#ifndef SPOT_TGBAPARSE_PARSEDECL_HH
# define SPOT_TGBAPARSE_PARSEDECL_HH

#include <string>
#include "tgbaparse.hh"
#include "location.hh"

# define YY_DECL \
  int tgbayylex (yystype *yylval, yy::Location *yylloc)
YY_DECL;

namespace spot
{
  int tgbayyopen(const std::string& name);
  void tgbayyclose();
}


// Gross kludge to compile yy::Parser in another namespace (tgbayy::)
// but still use yy::Location.  The reason is that Bison's C++
// skeleton does not support anything close to %name-prefix at the
// moment.  All parser are named yy::Parser which makes it somewhat
// difficult to define multiple parsers.
namespace tgbayy
{
  using namespace yy;
}
#define yy tgbayy



#endif // SPOT_TGBAPARSE_PARSEDECL_HH
