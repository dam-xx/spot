#include "sautparse/public.hh"


int main(int argc, char** argv)
{
  spot::saut_parse_error_list err;
  assert(argc > 1);
  spot::bdd_dict* dict = new spot::bdd_dict();
  spot::saut_parse_result* res =
    spot::saut_parse(argv[1], err, dict,
		     spot::ltl::default_environment::instance(), false);
  spot::format_saut_parse_errors(std::cerr, argv[1], err);
  delete res;
  delete dict;
  return 0;
}
