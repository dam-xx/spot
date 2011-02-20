#ifndef TRAD_HH_
# define TRAD_HH_

#include "tgba/public.hh"
#include "tgba/bdddict.hh"

class Trad
{
public:
  Trad (std::string name) { name_ = name; }
  virtual ~Trad () {}; 

  virtual spot::tgba* operator() (spot::ltl::formula* f, spot::bdd_dict* d) = 0;

  std::string name_get () const { return this->name_; }

protected:
  std::string name_;
};

#endif /* TRAD_HH_ */
