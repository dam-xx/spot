#include "gspn.hh"
#include "tgbaalgos/dotty.hh"

int 
main(int argc, char **argv)
  try
    {
      spot::gspn_interface gspn(argc, argv);
      
      spot::gspn_environment env;
      env.declare("obs");
      
      spot::bdd_dict* dict = new spot::bdd_dict();
      
      spot::tgba_gspn a(dict, env);
      
      spot::dotty_reachable(std::cout, &a);

      delete dict;
    }
  catch (spot::gspn_exeption e)
    { 
      std::cerr << e << std::endl;
      throw;
    }

