#include "gspn.hh"
#include "tgbaalgos/dotty.hh"

int
main(int argc, char **argv)
  try
    {
      spot::gspn_environment env;

      if (argc <= 2)
	{
	  std::cerr << "usage: " << argv[0] << " model props..." << std::endl;
	  exit(1);
	}

      while (argc > 2)
	{
	  env.declare(argv[argc - 1]);
	  --argc;
	}

      spot::gspn_interface gspn(2, argv);

      spot::bdd_dict* dict = new spot::bdd_dict();

      {
	spot::tgba_gspn a(dict, env);

	spot::dotty_reachable(std::cout, &a);
      }

      delete dict;
    }
  catch (spot::gspn_exeption e)
    {
      std::cerr << e << std::endl;
      throw;
    }
