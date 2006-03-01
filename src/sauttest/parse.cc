#include "sautparse/public.hh"


int main(int argc, char** argv)
{
  spot::saut_parse_error_list err;
  assert(argc > 1);
  spot::saut_parse(argv[1], err, 0,
		   spot::ltl::default_environment::instance(), false);
  spot::format_saut_parse_errors(std::cerr, argv[1], err);
  return 0;
}
