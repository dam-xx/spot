#ifndef TRAD_FM_HH_
# define TRAD_FM_HH_

#include "trad.hh"

class TradFm : public Trad
{
public:
  TradFm (std::string name, unsigned char bitmask);
  virtual ~TradFm ();

  static unsigned char optobin (std::vector<std::string> v);

  virtual spot::tgba* operator() (spot::ltl::formula* f, spot::bdd_dict* d);

protected:
  bool exprop_;
  bool symb_merge_;
  bool branching_;
  bool fair_loop_;
};

#endif /* TRAD_FM_HH_ */
