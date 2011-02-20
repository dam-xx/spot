#include "trad-fm.hh"

#include "tgbaalgos/ltl2tgba_fm.hh"

TradFm::TradFm (std::string name, unsigned char bitmask)
  : Trad (name),
    exprop_ (false),
    symb_merge_ (true),
    branching_ (false),
    fair_loop_ (false)
{
  if (bitmask & 1)
    exprop_ = true;

  if ((bitmask >> 1) & 1)
    symb_merge_ = false;

  if ((bitmask >> 2) & 1)
    branching_ = true;

  if ((bitmask >> 3) & 1)
    fair_loop_ = true;

  if ((bitmask >> 7) & 1)
    do_ = false;
}

TradFm::~TradFm ()
{

}

unsigned char
TradFm::optobin (std::vector<std::string> v)
{
  unsigned short res = 0;
  std::vector<std::string>::iterator i;

  for (i = v.begin (); i != v.end (); ++i)
  {
    if (*i == "exprop")
      res += (1 << 0);
    else if (*i == "symb")
      res += (1 << 1);
    else if (*i == "branching")
      res += (1 << 2);
    else if (*i == "fair")
      res += (1 << 3);
    else if (*i == "none")
      res += (1 << 7);

  }

  return res;
}

spot::tgba*
TradFm::operator() (spot::ltl::formula* f, spot::bdd_dict* d)
{
  return spot::ltl_to_tgba_fm (f,
			       d,
			       exprop_,
			       symb_merge_,
			       branching_,
			       fair_loop_);
}
