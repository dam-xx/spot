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
      for (bdd_dict::fv_map::const_iterator ai = d->acc_map.begin();
	   ai != d->acc_map.end(); ++ai)
	{
	  os_ << " \"";
	  ltl::to_string(ai->first, os_) << "\"";
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
