#include "trad-taa.hh"

#include "tgbaalgos/ltl2taa.hh"

TradTaa::TradTaa (std::string name)
  : Trad (name)
{

}

TradTaa::~TradTaa ()
{

}

spot::tgba*
TradTaa::operator() (spot::ltl::formula* f, spot::bdd_dict* d)
{
  return spot::ltl_to_taa (f, d);
}
