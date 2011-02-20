#ifndef BUILDER_HH_
# define BUILDER_HH_

#include <iostream>
#include <utility>
#include <map>
#include <vector>

#include "tgba/bdddict.hh"
#include "tgba/public.hh"
#include "ltlparse/public.hh"
#include "option-handler.hh"
#include "ltlparse/ltlfile.hh"

// maybe move it into the Builder
typedef std::pair <spot::tgba*, spot::tgba*> tgba_pair;
typedef std::map <std::string, tgba_pair> tgba_map;

class BuiltObj
{
public:
  BuiltObj (spot::ltl::formula* f,
	    spot::tgba* model,
	    tgba_map* m)
  {
    formula_ = f;
    model_ = model;
    m_ = m;
  };

  spot::ltl::formula* formula_get () { return this->formula_; };
  spot::tgba* model_get () { return this->model_; };
  tgba_map* m_get () { return this->m_; };

  ~BuiltObj ();

protected:
  spot::ltl::formula* formula_;
  spot::tgba* model_;
  tgba_map* m_;
};

/*
  Generate on the fly a BuiltObj
  see the above definition
*/
class Builder
{
public:
  Builder (OptionHandler& opth);
  ~Builder ();

  // return a built obj or NULL if there is no more formulas in the file
  BuiltObj* operator() ();

protected:
  spot::bdd_dict* dict_;
  spot::ltl::ltl_file* lf_;
  std::vector<Trad*> algo_;
};

#endif /* !BUILDER_HH_ */
