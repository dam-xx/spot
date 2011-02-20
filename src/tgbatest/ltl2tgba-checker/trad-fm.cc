#include "trad-fm.hh"

#include "tgbaalgos/ltl2tgba_fm.hh"

TradFm::TradFm (std::string name)
 : Trad (name)
{

}

TradFm::~TradFm ()
{

}

spot::tgba*
TradFm::operator() (spot::ltl::formula* f, spot::bdd_dict* d)
{
  return spot::ltl_to_tgba_fm (f, d);
}
