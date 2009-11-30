#include <iostream>
#include <cstring>

#include <saba/saba.hh>
#include <saba/sabacomplementtgba.hh>
#include <tgba/tgba.hh>
#include <tgbaparse/public.hh>
#include <tgba/tgbatba.hh>
#include <sabaalgos/sabadotty.hh>
#include <tgbaalgos/ltl2tgba_fm.hh>
#include <ltlparse/public.hh>

void usage(const std::string& argv0)
{
  std::cerr
    << "usage " << argv0 << " [options]" << std::endl
    << "options:" << std::endl
    << "-f formula          display the saba of !forumula"
    << std::endl;
}

int main(int argc, char* argv[])
{
  std::string formula;
  for (int i = 1; i < argc; ++i)
  {
    if (!strcmp(argv[i], "-f"))
    {
      if (i + 1 >= argc)
      {
        usage(argv[0]);
        return 1;
      }
      formula = argv[++i];
    }
    else
    {
      usage(argv[0]);
      return 1;
    }
  }

  if (formula.empty())
  {
    usage(argv[0]);
    return 1;
  }

  spot::bdd_dict* dict = new spot::bdd_dict();
  spot::tgba* a;
  spot::ltl::formula* f1 = 0;

  spot::ltl::parse_error_list p1;
  f1 = spot::ltl::parse(formula, p1);
  if (spot::ltl::format_parse_errors(std::cerr, formula, p1))
    return 2;

  a = spot::ltl_to_tgba_fm(f1, dict);

  spot::saba_complement_tgba* complement =
    new spot::saba_complement_tgba(a);

  spot::saba_dotty_reachable(std::cout, complement);

  delete complement;
  delete a;
  f1->destroy();
  delete dict;
}
