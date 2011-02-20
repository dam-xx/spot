#ifndef TRAD_LACIM_HH_
# define TRAD_LACIM_HH_

#include "trad.hh"

class TradLacim : public Trad
{
public:
  TradLacim (std::string name);
  virtual ~TradLacim ();

  virtual spot::tgba* operator() (spot::ltl::formula* f, spot::bdd_dict* d);
};


#endif /* TRAD_LACIM_HH_ */
