#include "tgba/tgba.hh"
#include "dotty.hh"
#include "tgba/bddprint.hh"
#include "reachiter.hh"

namespace spot
{

  class dotty_bfs : public tgba_reachable_iterator_breadth_first
  {
  public:
    dotty_bfs(const tgba* a, std::ostream& os)
      : tgba_reachable_iterator_breadth_first(a), os_(os)
    {
    }

    void
    start()
    {
      os_ << "digraph G {" << std::endl;
      os_ << "  0 [label=\"\", style=invis, height=0]" << std::endl;
      os_ << "  0 -> 1" << std::endl;
    }

    void
    end()
    {
      os_ << "}" << std::endl;
    }

    void
    process_state(const state* s, int n, tgba_succ_iterator*)
    {
      os_ << "  " << n << " [label=\""
	  << automata_->format_state(s) << "\"]" << std::endl;
    }

    void
    process_link(int in, int out, const tgba_succ_iterator* si)
    {
      os_ << "  " << in << " -> " << out << " [label=\"";
      bdd_print_set(os_, automata_->get_dict(),
		    si->current_condition()) << "\\n";
      bdd_print_set(os_, automata_->get_dict(),
		    si->current_accepting_conditions()) << "\"]" << std::endl;
    }

  private:
    std::ostream& os_;
  };

  std::ostream&
  dotty_reachable(std::ostream& os, const tgba* g)
  {
    dotty_bfs d(g, os);
    d.run();
    return os;
  }


}
