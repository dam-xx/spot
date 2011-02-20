#ifndef TRAD_FM_HH_
# define TRAD_FM_HH_

#include "trad.hh"

class TradFm : public Trad
{
public:
  TradFm (std::string name);
  virtual ~TradFm ();

  virtual spot::tgba* operator() (spot::ltl::formula* f, spot::bdd_dict* d);
};

#endif /* TRAD_FM_HH_ */
