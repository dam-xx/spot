#include "trad-lacim.hh"

#include "tgbaalgos/ltl2tgba_lacim.hh"

TradLacim::TradLacim (std::string name)
  : Trad (name)
{
  // Handle options here
}

TradLacim::~TradLacim ()
{
  // Destroy something here
}

spot::tgba*
TradLacim::operator() (spot::ltl::formula* f, spot::bdd_dict* d)
{
  return spot::ltl_to_tgba_lacim (f, d);
}
