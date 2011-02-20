#include "trad-lacim.hh"

#include "tgbaalgos/ltl2tgba_lacim.hh"

TradLacim::TradLacim (std::string name, unsigned char bitmask)
  : Trad (name)
{
  if ((bitmask >> 7) & 1)
    do_ = false;
}

TradLacim::~TradLacim ()
{
  // Destroy something here
}

unsigned char
TradLacim::optobin (std::vector<std::string> v)
{
  unsigned char res = 0;
  std::vector<std::string>::iterator i;

  for (i = v.begin (); i != v.end (); ++i)
    {
      if (*i == "none")
	res += (1 << 7);
    }
  return res;
}

spot::tgba*
TradLacim::operator() (spot::ltl::formula* f, spot::bdd_dict* d)
{
  return spot::ltl_to_tgba_lacim (f, d);
}
