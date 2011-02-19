#include <iostream>
#include "option-handler.hh"
#include "builder.hh"

#include "ltlvisit/tostring.hh"

int
main (int argc, char** argv)
{
  OptionHandler h (argc, argv);

  if (h.vm_get ().count ("help"))
    std::cout << h.desc_get () << std::endl;

  Builder b (h);

  BuiltObj* o1;

  o1 = b();

  std::cout << spot::ltl::to_string (o1->formula_get ()) << std::endl;

  std::cout << "toto" << std::endl;
}
