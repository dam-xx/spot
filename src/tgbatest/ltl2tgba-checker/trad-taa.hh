#ifndef TRAD_TAA_HH_
# define TRAD_TAA_HH_

#include "trad.hh"

class TradTaa : public Trad
{
public:
  TradTaa (std::string name, unsigned char bitmask);
  ~TradTaa ();

  static unsigned char optobin (std::vector<std::string> v);

  virtual spot::tgba* operator() (spot::ltl::formula* f, spot::bdd_dict* d);

protected:
  bool refined_;
};


#endif /* TRAD_TAA_HH_ */
