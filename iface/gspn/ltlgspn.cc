#include "gspn.hh"
#include "ltlparse/public.hh"
#include "ltlvisit/destroy.hh"
#include "tgba/tgbatba.hh"
#include "tgba/tgbaproduct.hh"
#include "tgbaalgos/ltl2tgba_lacim.hh"
#include "tgbaalgos/ltl2tgba_fm.hh"
#include "tgbaalgos/magic.hh"
#include "tgbaalgos/emptinesscheck.hh"

void
syntax(char* prog)
{
  std::cerr << "Usage: "<< prog
	    << " [OPTIONS...] model formula props..."   << std::endl
	    << std::endl
	    << "  -c  compute a counter example" << std::endl
	    << "      (instead of just checking for emptiness)" << std::endl
	    << std::endl
	    << "  -e  use Couvreur's emptiness-check (default)" << std::endl
	    << "  -m  degeneralize and perform a magic-search" << std::endl
	    << std::endl
            << "  -l  use Couvreur's LaCIM algorithm for translation (default)"
	    << std::endl
            << "  -f  use Couvreur's FM algorithm for translation"
	    << std::endl;
  exit(2);
}

int
main(int argc, char **argv)
  try
    {
      int formula_index = 1;
      enum { Couvreur, Magic } check = Couvreur;
      enum { Lacim, Fm } trans = Lacim;
      bool compute_counter_example = false;

      spot::gspn_environment env;

      while (formula_index < argc && *argv[formula_index] == '-')
	{
	  if (!strcmp(argv[formula_index], "-c"))
	    {
	      compute_counter_example = true;
	    }
	  else if (!strcmp(argv[formula_index], "-e"))
	    {
	      check = Couvreur;
	    }
	  else if (!strcmp(argv[formula_index], "-m"))
	    {
	      check = Magic;
	    }
	  else if (!strcmp(argv[formula_index], "-l"))
	    {
	      trans = Lacim;
	    }
	  else if (!strcmp(argv[formula_index], "-f"))
	    {
	      trans = Fm;
	    }
	  else
	    {
	      syntax(argv[0]);
	    }
	  ++formula_index;
	}
      if (argc < formula_index + 3)
	syntax(argv[0]);


      while (argc > formula_index + 2)
	{
	  env.declare(argv[argc - 1]);
	  --argc;
	}

      spot::ltl::parse_error_list pel;
      spot::ltl::formula* f = spot::ltl::parse(argv[formula_index + 1],
					       pel, env);

      if (spot::ltl::format_parse_errors(std::cerr,
					 argv[formula_index + 1], pel))
	exit(2);

      argv[1] = argv[formula_index];
      spot::gspn_interface gspn(2, argv);
      spot::bdd_dict* dict = new spot::bdd_dict();

      spot::tgba* a_f = 0;
      switch (trans)
	{
	case Fm:
	  a_f = spot::ltl_to_tgba_fm(f, dict);
	  break;
	case Lacim:
	  a_f = spot::ltl_to_tgba_lacim(f, dict);
	  break;
	}
      spot::ltl::destroy(f);

      spot::tgba* model        = new spot::tgba_gspn(dict, env);
      spot::tgba_product* prod = new spot::tgba_product(model, a_f);

      switch (check)
	{
	case Couvreur:
	  {
	    spot::emptiness_check ec;
	    bool res = ec.tgba_emptiness_check(prod);
	    if (!res)
	      {
		if (compute_counter_example)
		  {
		    ec.counter_example(prod);
		    ec.print_result(std::cout, prod, model);
		  }
		else
		  {
		    std::cout << "non empty" << std::endl;
		  }
		exit(1);
	      }
	    else
	      {
		std::cout << "empty" << std::endl;
	      }
	  }
	  break;
	case Magic:
	  {
	    spot::tgba_tba_proxy* d  = new spot::tgba_tba_proxy(prod);
	    spot::magic_search ms(d);

	    if (ms.check())
	      {
		if (compute_counter_example)
		  ms.print_result (std::cout, model);
		else
		  std::cout << "non-empty" << std::endl;
		exit(1);
	      }
	    else
	      {
		std::cout << "empty" << std::endl;
	      }
	    delete d;
	  }
	}
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
