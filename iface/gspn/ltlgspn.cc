#include "gspn.hh"
#include "ltlparse/public.hh"
#include "ltlvisit/destroy.hh"
#include "tgba/tgbatba.hh"
#include "tgba/tgbaproduct.hh"
#include "tgbaalgos/ltl2tgba_lacim.hh"
#include "tgbaalgos/ltl2tgba_fm.hh"
#include "tgbaalgos/magic.hh"
#include "tgbaalgos/emptinesscheck.hh"

int
main(int argc, char **argv)
  try
    {
      spot::gspn_environment env;

      if (argc <= 3)
	{
	  std::cerr << "usage: " << argv[0]
		    << " model formula props..." << std::endl;
	  exit(1);
	}

      while (argc > 3)
	{
	  env.declare(argv[argc - 1]);
	  --argc;
	}

      spot::ltl::parse_error_list pel;
      spot::ltl::formula* f = spot::ltl::parse(argv[2], pel, env);

      if (spot::ltl::format_parse_errors(std::cerr, argv[2], pel))
	exit(1);

      spot::gspn_interface gspn(2, argv);
      spot::bdd_dict* dict = new spot::bdd_dict();

#if FM
      spot::tgba* a_f = spot::ltl_to_tgba_fm(f, dict);
#else
      spot::tgba* a_f = spot::ltl_to_tgba_lacim(f, dict);
#endif
      spot::ltl::destroy(f);

      spot::tgba* model        = new spot::tgba_gspn(dict, env);
      spot::tgba_product* prod = new spot::tgba_product(model, a_f);
#if CEC
      {
	spot::emptiness_check empty_check;
	bool res = empty_check.tgba_emptiness_check(prod);
	if (!res)
	  {
	    empty_check.counter_example(prod);
	    exit(1);
	  }
	else
	  {
	    std::cout << "not found";
	  }
      }
#else
      spot::tgba_tba_proxy* d  = new spot::tgba_tba_proxy(prod);
      {
	spot::magic_search ms(d);

	if (ms.check())
	  {
	    ms.print_result (std::cout, model);
	    exit(1);
	  }
	else
	  {
	    std::cout << "not found";
	  }
      }
      delete d;
#endif
      delete prod;
      delete model;
      delete a_f;
      delete dict;
    }
  catch (spot::gspn_exeption e)
    {
      std::cerr << e << std::endl;
      throw;
    }
