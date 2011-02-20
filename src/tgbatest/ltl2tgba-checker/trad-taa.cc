#include "trad-taa.hh"

#include "tgbaalgos/ltl2taa.hh"

TradTaa::TradTaa (std::string name, unsigned char bitmask)
  : Trad (name)
{
  if ((bitmask >> 7) & 1)
    refined_ = true;
  else if ((bitmask >> 7) & 1)
    do_ = false;
}

TradTaa::~TradTaa ()
{

}

unsigned char
TradTaa::optobin (std::vector<std::string> v)
{
  unsigned char ret = 0;
  std::vector <std::string>::iterator i;

  for (i = v.begin (); i != v.end (); ++i)
  {
    if (*i == "refined")
      ret += (1 << 0);
    else if (*i == "none")
	ret += (1 << 7);
  }

  return ret;
}

spot::tgba*
TradTaa::operator() (spot::ltl::formula* f, spot::bdd_dict* d)
{
  return spot::ltl_to_taa (f, d, refined_);
}
