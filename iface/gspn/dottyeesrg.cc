#include "eesrg.hh"
#include "tgbaalgos/dotty.hh"
#include "tgba/tgbaexplicit.hh"
#include "tgbaparse/public.hh"

int
main(int argc, char **argv)
  try
    {
      spot::gspn_environment env;

      if (argc <= 3)
	{
	  std::cerr << "usage: " << argv[0] << " model automata props..."
		    << std::endl;
	  exit(1);
	}

      while (argc > 3)
	env.declare(argv[--argc]);

      spot::gspn_eesrg_interface gspn(2, argv);
      spot::bdd_dict* dict = new spot::bdd_dict();

      spot::tgba_parse_error_list pel1;
      spot::tgba_explicit* control = spot::tgba_parse(argv[--argc], pel1,
						      dict, env);
      if (spot::format_tgba_parse_errors(std::cerr, pel1))
	return 2;

      {
	spot::tgba_gspn_eesrg a(dict, env, control);

	spot::dotty_reachable(std::cout, &a);
      }

      delete control;
      delete dict;
    }
  catch (spot::gspn_exeption e)
    {
      std::cerr << e << std::endl;
      throw;
    }
