// Copyright (C) 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
// département Systèmes Répartis Coopératifs (SRC), Université Pierre
// et Marie Curie.
//
// This file is part of Spot, a model checking library.
//
// Spot is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Spot is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Spot; see the file COPYING.  If not, write to the Free
// Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#ifndef SPOT_TGBAALGOS_MINIMALCE_HH
# define SPOT_TGBAALGOS_MINIMALCE_HH

#include "misc/hash.hh"
#include <list>
#include <utility>
#include <ostream>
#include <sstream>
#include "tgba/tgbatba.hh"
#include "tgba/bddprint.hh"
//#include "tgbaalgos/gtec/ce.hh"

#include <time.h>

namespace spot
{

  enum search_opt
    {
      magic     = 0,
      nested    = 1,
      my_nested = 2
    };

  namespace ce
  {

    typedef std::pair<const state*, bdd> state_ce;
    typedef std::list<state_ce> l_state_ce;

    ///////////////////////////////////////////////////////////////////////////
    // Class counter example.

    class counter_example
    {
    public :
      counter_example(const tgba* a);
      ~counter_example();

      void build_cycle(const state* x);
      int size();
      std::ostream& print(std::ostream& os) const;
      bdd_dict* get_dict() const;

      /// \brief Project a counter example on a tgba.
      void project_ce(const tgba* aut, std::ostream& os = std::cout);
      /// \brief Build a tgba from a counter example.
      tgba* ce2tgba();

      l_state_ce prefix;
      l_state_ce cycle;
      const tgba* automata_;

    };

  }

  /////////////////////////////////////////////////////////////////////////////
  // The base interface to build an emptiness check algorithm
  class emptiness_search
  {
  protected:
    emptiness_search();

  public:
    virtual ~emptiness_search();
    virtual ce::counter_example* check() = 0;
    /// \brief Print Stat.
    virtual std::ostream& print_stat(std::ostream& os) const;
  };

  /////////////////////////////////////////////////////////////////////////////
  // Perform a minimal search

  class minimalce_search: public emptiness_search
  {
  public:
    //minimalce_search(const tgba_tba_proxy *a, bool mode = false);
    minimalce_search(const tgba_tba_proxy *a, int opt = nested);

    virtual ~minimalce_search();

    /// \brief Find the shortest counter example.
    virtual ce::counter_example* check();

    /// \brief Find a counter example shorter than \a min_ce.
    //ce::counter_example* check(ce::counter_example* min_ce);

    //ce::counter_example* find();

    /// \brief Print Stat.
    std::ostream& print_stat(std::ostream& os) const;
    std::ostream& print_result(std::ostream& os,
			       const tgba* restrict = 0) const;

    //ce::counter_example* get_minimal_cyle() const;
    //ce::counter_example* get_minimal_prefix() const;

  private:

    /// \brief Minimisation is implemented on the magic search algorithm.
    struct magic
    {
      bool seen_without : 1;
      bool seen_with    : 1;
      bool seen_path    : 1;
      unsigned int depth;
    };

    struct magic_state
    {
      const state* s;
      bool m;
    };

    enum search_mode
      {
	normal = 0,
	careful = 1
      };
    //int mode;


    typedef std::pair<magic_state, tgba_succ_iterator*> state_iter_pair;
    typedef std::list<state_iter_pair> stack_type;
    stack_type stack;		///< Stack of visited states on the path.

    typedef std::list<bdd> tstack_type;
    /// \brief Stack of transitions.
    ///
    /// This is an addition to the data from the paper.
    tstack_type tstack;

    typedef Sgi::hash_map<const state*, magic,
			  state_ptr_hash, state_ptr_equal> hash_type;
    hash_type h;		///< Map of visited states.

    /// Append a new state to the current path.
    bool push(const state* s, bool m);
    /// Check whether we already visited \a s with the Magic bit set to \a m.
    bool has(const state* s, bool m) const;
    /// Check if \a s is in the path.
    bool exist_path(const state* s) const;
    /// Return the depth of the state \a s in stack.
    int depth_path(const state* s) const;

    void build_counter();

    const tgba_tba_proxy* a;	///< The automata to check.
    /// The state for which we are currently seeking an SCC.
    const state* x;
    /// \brief Active the nested search which produce a
    /// smaller counter example.
    bool nested_;
    /// \brief Active the nested bis search which produce a
    /// smaller counter example.
    const state* x_bis;
    bool my_nested_;
    bool accepted_path_;
    int accepted_depth_;

    unsigned int Maxsize;

    ce::counter_example* counter_;
    std::list<ce::counter_example*> l_ce;

    ///////////////////////////////////////////////////////////////////
    /*
    //typedef std::pair<int, tgba_succ_iterator*> state_iter_pair;
    typedef Sgi::hash_map<const state*, int,
			  state_ptr_hash, state_ptr_equal> hash_type;
    hash_type h_lenght;		///< Map of visited states.

    typedef std::pair<const state*, tgba_succ_iterator*> state_pair;
    //typedef std::pair<const state*, bdd> state_pair;
    typedef std::list<state_pair> stack_type;
    //typedef std::list<const state*> stack_type;
    stack_type stack;		///< Stack of visited states on the path.

    const tgba_tba_proxy* a;	///< The automata to check.
    /// The state for which we are currently seeking an SCC.
    //const state* x;
    ce::counter_example* min_ce;
    std::list<ce::counter_example*> l_ce;
    int nb_found;
    bool mode_;
    clock_t tps_;

    /// \brief Perform the minimal search as explain in
    /// @InProceedings(GaMoZe04spin,
    /// Author = "Gastin, P. and Moro, P. and Zeitoun, M.",
    /// Title = "Minimization of counterexamples in {SPIN}",
    /// BookTitle = "Proceedings of the 11th SPIN Workshop (SPIN'04)",
    /// Editor = "Graf, S. and Mounier, L.",
    /// Publisher = SPRINGER,
    /// Series = LNCS,
    /// Number = 2989,
    /// Year = 2004,
    /// Pages = "92-108")
    void recurse_find(const state* it,
		      std::ostringstream& os,
		      int mode = normal);
    bool closes_accepting(const state* s,
			  int detph,
			  std::ostringstream& os) const;
    int in_stack(const state* s, std::ostringstream& os) const;

    /// Save the current path in stack as a counter example.
    /// this counter example is the minimal that we have found yet.
    void save_counter(const state* s, std::ostringstream& os);
    */
  };

}

#endif // SPOT_TGBAALGOS_MINIMALCE_HH
