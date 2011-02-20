#ifndef TRAD_TAA_HH_
# define TRAD_TAA_HH_

#include "trad.hh"

class TradTaa : public Trad
{
public:
  TradTaa (std::string name);
  ~TradTaa ();

  virtual spot::tgba* operator() (spot::ltl::formula* f, spot::bdd_dict* d);
};


#endif /* TRAD_TAA_HH_ */
