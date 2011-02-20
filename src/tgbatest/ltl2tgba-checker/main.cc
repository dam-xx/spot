#include <iostream>
#include <sys/time.h>
#include "option-handler.hh"
#include "builder.hh"
#include "tgbaalgos/emptiness.hh"
#include "ltlvisit/tostring.hh"
#include "ltlvisit/apcollect.hh"
#include "algos.hh"


int
main (int argc, char** argv)
{
  OptionHandler h (argc, argv);

  if (h.vm_get ().count ("help"))
    std::cout << h.desc_get () << std::endl;

  Builder b (h);

  BuiltObj* o1;

  // spot::emptiness_check_instantiator* inst;
  // const char *err;
  // std::string s ("Cou99");
  // inst = spot::emptiness_check_instantiator::construct(s.c_str(), &err);

  unsigned inter_success = 0;
  unsigned cons_success = 0;
  unsigned cross_success = 0;
  unsigned i = 0;

  std::cout << "Seed used: " << h.seed_get() << std::endl;
  struct timeval start;
  TimerReset(start);
  while (o1 = b())
  {
    spot::ltl::formula* f = o1->formula_get();
    std::cout << "Formula " << ++i << ": "
              << spot::ltl::to_string (f) << std::endl;
    bool intersection_result = check_intersection(o1->m_get(),
						  b.emptiness_get ());

    if (intersection_result)
      ++inter_success;
    bool cross_comparison_result = check_cross_comparison(o1->m_get(),
                                                          o1->model_get(),
                                                          b.emptiness_get ());

    if (cross_comparison_result)
      ++cross_success;
    bool consistency_result = check_consistency(o1->m_get(),
                                                o1->model_get());

    if (consistency_result)
      ++cons_success;

    std::cout << "Check intersection: "
              << std::boolalpha << intersection_result << std::endl;
    std::cout << "Check consistency: "
              << std::boolalpha << consistency_result << std::endl;
    std::cout << "Check cross-comparison: "
              << std::boolalpha << cross_comparison_result << std::endl;
    std::cout << std::endl;
  }

  double elapsed = TimerGetElapsedTime(start);
  std::cout << std::endl
            << "Summary: " << std::endl
            << i << " formulae parsed." << std::endl
            << "Intersection check: " << inter_success << "/" << i << std::endl
            << "Consistency check: " << cons_success << "/" << i << std::endl
            << "Cross-comparison check: " << cross_success << "/" << i
            << std::endl
            << "Computation time: " << elapsed << std::endl;
}
