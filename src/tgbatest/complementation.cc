#include <iomanip>
#include <iostream>
#include "tgbaalgos/dotty.hh"
#include "tgbaparse/public.hh"
#include "tgba/tgbaproduct.hh"
#include "tgbaalgos/gtec/gtec.hh"
#include "tgbaalgos/ltl2tgba_fm.hh"
#include "ltlparse/public.hh"
#include "tgbaalgos/stats.hh"
#include "tgbaalgos/emptiness.hh"
#include "ltlast/unop.hh"
#include "tgbaalgos/stats.hh"
#include "tgbaalgos/emptiness_stats.hh"
#include "ltlvisit/destroy.hh"
#include "ltlvisit/clone.hh"
#include "tgba/tgbatba.hh"

#include "tgba/tgbacomplement.hh"

void usage(const char* prog)
{
  std::cout << "usage: " << prog << " [options]" << std::endl;
  std::cout << "with options" << std::endl
            << "-s     buchi_automaton  display the safra automaton"
            << std::endl
            << "-a     buchi_automaton  display the complemented automaton"
            << std::endl
            << "-astat buchi_automaton  statistics for !a" << std::endl
            << "-fstat formula          statistics for !A_f" << std::endl
            << "-f     formula          test !A_f and !A_!f" << std::endl;
}

int main(int argc, char* argv[])
{
  char *file = 0;
  bool print_safra = false;
  bool print_automaton = false;
  bool check = false;
  int return_value = 0;
  bool stats = false;
  bool formula = false;
  bool automaton = false;

  if (argc < 3)
  {
    usage(argv[0]);
    return 1;
  }

  for (int i = 1; i < argc; ++i)
  {
    if (argv[i][0] == '-')
    {
      if (strcmp(argv[i] + 1, "astat") == 0)
      {
        stats = true;
        automaton = true;
        continue;
      }

      if (strcmp(argv[i] + 1, "fstat") == 0)
      {
        stats = true;
        formula = true;
        continue;
      }

      switch (argv[i][1])
      {
        case 's': print_safra = true; break;
        case 'a': print_automaton = true; break;
        case 'f': check = true; break;
        default:
          std::cerr << "unrecognized option `-" << argv[i][1]
                    << "'" << std::endl;
          return 2;
      }
    }
    else
      file = argv[i];
  }

  if (file == 0)
  {
    usage(argv[0]);
    return 1;
  }

  spot::bdd_dict* dict = new spot::bdd_dict();
  if (print_automaton || print_safra)
  {
    spot::ltl::environment& env(spot::ltl::default_environment::instance());
    spot::tgba_parse_error_list pel;
    spot::tgba_explicit* a = spot::tgba_parse(file, pel, dict, env);
    if (spot::format_tgba_parse_errors(std::cerr, file, pel))
      return 2;

    spot::tgba_complement* complement = new spot::tgba_complement(a);

    if (print_automaton)
      spot::dotty_reachable(std::cout, complement);

    if (print_safra)
      spot::display_safra(complement);
    delete complement;
    delete a;
  }
  else if (stats)
  {
    spot::tgba* a;
    spot::ltl::formula* f1 = 0;

    if (formula)
    {
      spot::ltl::parse_error_list p1;
      f1 = spot::ltl::parse(file, p1);

      if (spot::ltl::format_parse_errors(std::cerr, file, p1))
        return 2;

      a = spot::ltl_to_tgba_fm(f1, dict);
    }
    else
    {
      spot::tgba_parse_error_list pel;
      spot::ltl::environment& env(spot::ltl::default_environment::instance());
      a = spot::tgba_parse(file, pel, dict, env);
      if (spot::format_tgba_parse_errors(std::cerr, file, pel))
        return 2;
    }

    spot::tgba_complement* complement = new spot::tgba_complement(a);

    spot::tgba_statistics a_size =  spot::stats_reachable(a);
    std::cout << "Original: "
              << a_size.states << ", "
              << a_size.transitions << std::endl;

    spot::tgba *buchi = new spot::tgba_sba_proxy(a);
    a_size =  spot::stats_reachable(buchi);
    std::cout << "Buchi: "
              << a_size.states << ", "
              << a_size.transitions << std::endl;
    delete buchi;

    spot::tgba_statistics b_size =  spot::stats_reachable(complement);
    std::cout << "Complement: "
              << b_size.states << ", "
              << b_size.transitions << std::endl;

    delete complement;
    delete a;
    if (formula)
    {
      spot::ltl::formula* nf1 =
        spot::ltl::unop::instance(spot::ltl::unop::Not,
                                  spot::ltl::clone(f1));
      spot::tgba* a2 = spot::ltl_to_tgba_fm(nf1, dict);
      spot::tgba_statistics a_size =  spot::stats_reachable(a2);
      std::cout << "Not Formula: "
                << a_size.states << ", "
                << a_size.transitions << std::endl;

      delete a2;
      spot::ltl::destroy(f1);
      spot::ltl::destroy(nf1);
    }
  }
  else
  {
    spot::ltl::parse_error_list p1;
    spot::ltl::formula* f1 = spot::ltl::parse(file, p1);

    if (spot::ltl::format_parse_errors(std::cerr, file, p1))
      return 2;

    spot::tgba* Af = spot::ltl_to_tgba_fm(f1, dict);
    spot::ltl::formula* nf1 = spot::ltl::unop::instance(spot::ltl::unop::Not,
                                                        spot::ltl::clone(f1));
    spot::tgba* Anf = spot::ltl_to_tgba_fm(nf1, dict);
    spot::tgba_complement* nAf = new spot::tgba_complement(Af);
    spot::tgba_complement* nAnf = new spot::tgba_complement(Anf);
    spot::tgba* prod = new spot::tgba_product(nAf, nAnf);
    spot::emptiness_check* ec = spot::couvreur99(prod);
    spot::emptiness_check_result* res = ec->check();
    spot::tgba_statistics a_size =  spot::stats_reachable(ec->automaton());
    std::cout << std::right << std::setw(10)
              << a_size.states << ", "
              << std::right << std::setw(10)
              << a_size.transitions << ", "
              << ec->automaton()->number_of_acceptance_conditions();
    if (res)
    {
      std::cout << ", FAIL";
      return_value = 1;
    }
    else
      std::cout << ", OK";
    std::cout << std::endl;

    delete res;
    delete ec;
    delete prod;
    delete nAf;
    delete Af;
    delete nAnf;
    delete Anf;

    spot::ltl::destroy(nf1);
    spot::ltl::destroy(f1);

  }

  delete dict;

  return return_value;
}
