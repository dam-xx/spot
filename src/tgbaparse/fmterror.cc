#include "public.hh"

namespace spot
{
  bool
  format_tgba_parse_errors(std::ostream& os,
			   tgba_parse_error_list& error_list)
  {
    bool printed = false;
    spot::tgba_parse_error_list::iterator it;
    for (it = error_list.begin(); it != error_list.end(); ++it)
      {
	if (it->first.begin.filename != "")
	  os << it->first << ": ";
	os << it->second << std::endl;
	printed = true;
      }
    return printed;
  }
}
