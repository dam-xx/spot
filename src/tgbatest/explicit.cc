#include <iostream>
#include "ltlenv/defaultenv.hh"
#include "tgba/tgbaexplicit.hh"
#include "tgbaalgos/dotty.hh"

int
main()
{
  spot::ltl::default_environment& e =
    spot::ltl::default_environment::instance();
  spot::tgba_explicit a;

  typedef spot::tgba_explicit::transition trans;

  trans* t1 = a.create_transition("state 0", "state 1");
  trans* t2 = a.create_transition("state 1", "state 2");
  trans* t3 = a.create_transition("state 2", "state 0");
  a.add_condition(t2, e.require("a"));
  a.add_condition(t3, e.require("b"));
  a.add_condition(t3, e.require("c"));
  a.declare_accepting_condition(e.require("p"));
  a.declare_accepting_condition(e.require("q"));
  a.declare_accepting_condition(e.require("r"));
  a.add_accepting_condition(t1, e.require("p"));
  a.add_accepting_condition(t1, e.require("q"));
  a.add_accepting_condition(t2, e.require("r"));

  spot::dotty_reachable(std::cout, a);
}
