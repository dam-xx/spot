#include "tgba/tgba.hh"
#include "save.hh"
#include "tgba/bddprint.hh"
#include "ltlvisit/tostring.hh"
#include "reachiter.hh"

namespace spot
{

  class save_bfs : public tgba_reachable_iterator_breadth_first
  {
  public:
    save_bfs(const tgba* a, std::ostream& os)
      : tgba_reachable_iterator_breadth_first(a), os_(os)
    {
    }

    void
    start()
    {
      const bdd_dict* d = automata_->get_dict();
      os_ << "acc =";

      bdd acc = automata_->all_accepting_conditions();
      while (acc != bddfalse)
	{
	  bdd cube = bdd_satone(acc);
	  acc -= cube;
	  while (cube != bddtrue)
	    {
	      assert(cube != bddfalse);
	      // Display the first variable that is positive.
	      // There should be only one per satisfaction.
	      if (bdd_high(cube) != bddfalse)
		{
		  int v = bdd_var(cube);
		  bdd_dict::vf_map::const_iterator vi =
		    d->acc_formula_map.find(v);
		  assert(vi != d->acc_formula_map.end());
		  os_ << " \"";
		  ltl::to_string(vi->second, os_) << "\"";
		  break;
		}
	      cube = bdd_low(cube);
	    }
	}
      os_ << ";" << std::endl;
    }

    void
    process_state(const state* s, int, tgba_succ_iterator* si)
    {
      const bdd_dict* d = automata_->get_dict();
      std::string cur = automata_->format_state(s);
      for (si->first(); !si->done(); si->next())
	{
	  state* dest = si->current_state();
	  os_ << "\"" << cur << "\", \""
	      << automata_->format_state(dest) << "\", ";
	  bdd_print_sat(os_, d, si->current_condition()) << ",";
	  bdd_print_acc(os_, d, si->current_accepting_conditions());
	  os_ << ";" << std::endl;
	}
    }

  private:
    std::ostream& os_;
  };


  std::ostream&
  tgba_save_reachable(std::ostream& os, const tgba* g)
  {
    save_bfs b(g, os);
    b.run();
    return os;
  }
}
