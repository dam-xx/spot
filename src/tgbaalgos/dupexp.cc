#include "dupexp.hh"
#include <sstream>
#include <string>
#include <map>
#include "reachiter.hh"

namespace spot {

  template <class T>
  class dupexp_iter: public T
  {
  public:
    dupexp_iter(const tgba* a)
      : T(a), out_(new tgba_explicit(a->get_dict()))
    {
    }

    tgba_explicit*
    result()
    {
      return out_;
    }

    void
    process_state(const state* s, int n, tgba_succ_iterator*)
    {
      std::ostringstream os;
      os << "(#" << n << ") " << automata_->format_state(s);
      name_[n] = os.str();
    }

    std::string
    declare_state(const state* s, int n)
    {
      std::string str;
      name_map_::const_iterator i = name_.find(n);
      if (i == name_.end())
	{
	  std::ostringstream os;
	  os << "(#" << n << ") " << automata_->format_state(s);
	  name_[n] = str = os.str();
	}
      else
	{
	  str = i->second;
	}
      delete s;
      return str;
    }

    void
    process_link(int in, int out, const tgba_succ_iterator* si)
    {
      // We might need to format out before process_state is called.
      name_map_::const_iterator i = name_.find(out);
      if (i == name_.end())
	{
	  const state* s = si->current_state();
	  process_state(s, out, 0);
	  delete s;
	}

      tgba_explicit::transition* t =
	out_->create_transition(name_[in], name_[out]);
      out_->add_conditions(t, si->current_condition());
      out_->add_accepting_conditions(t, si->current_accepting_conditions());
    }

  private:
    tgba_explicit* out_;
    typedef std::map<int, std::string> name_map_;
    std::map<int, std::string> name_;
  };

  tgba_explicit*
  tgba_dupexp_bfs(const tgba* aut)
  {
    dupexp_iter<tgba_reachable_iterator_breadth_first> di(aut);
    di.run();
    return di.result();
  }

  tgba_explicit*
  tgba_dupexp_dfs(const tgba* aut)
  {
    dupexp_iter<tgba_reachable_iterator_depth_first> di(aut);
    di.run();
    return di.result();
  }

}
