#ifndef TRAD_LACIM_HH_
# define TRAD_LACIM_HH_

#include "trad.hh"

class TradLacim : public Trad
{
public:
  TradLacim (std::string name, unsigned char bitmask);
  virtual ~TradLacim ();

  static unsigned char optobin (std::vector<std::string> v);

  virtual spot::tgba* operator() (spot::ltl::formula* f, spot::bdd_dict* d);
};


#endif /* TRAD_LACIM_HH_ */
