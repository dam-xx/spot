// Copyright (C) 2003, 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
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

/// FIXME: Add
/// - the optimisation based on weights (a weight by accepting conditions),
/// - the computation of a counter example if detected.
/// - a bit-state hashing version.

//#define TRACE

#ifdef TRACE
#include <iostream>
#include "tgba/bddprint.hh"
#endif

#include <cassert>
#include <list>
#include <iterator>
#include "misc/hash.hh"
#include "tgba/tgba.hh"
#include "emptiness.hh"
#include "magic.hh"
#include "tau03opt.hh"

namespace spot
{
  namespace
  {
    enum color {WHITE, CYAN, BLUE};

    /// \brief Emptiness checker on spot::tgba automata having at most one
    /// accepting condition (i.e. a TBA).
    template <typename heap>
    class tau03_opt_search : public emptiness_check
    {
    public:
      /// \brief Initialize the search algorithm on the automaton \a a
      tau03_opt_search(const tgba *a, size_t size)
        : h(size), a(a), all_acc(a->all_acceptance_conditions())
      {
        assert(a->number_of_acceptance_conditions() > 0);
      }

      virtual ~tau03_opt_search()
      {
        // Release all iterators on the stacks.
        while (!st_blue.empty())
          {
            h.pop_notify(st_blue.front().s);
            delete st_blue.front().it;
            st_blue.pop_front();
          }
        while (!st_red.empty())
          {
            h.pop_notify(st_red.front().s);
            delete st_red.front().it;
            st_red.pop_front();
          }
      }

      /// \brief Perform a Magic Search.
      ///
      /// \return non null pointer iff the algorithm has found an
      /// accepting path.
      virtual emptiness_check_result* check()
      {
        if (!st_red.empty())
          {
            assert(!st_blue.empty());
            return 0;
          }
        assert(st_blue.empty());
        nbn = nbt = 0;
        sts = mdp = 0;
        const state* s0 = a->get_init_state();
        ++nbn;
        h.add_new_state(s0, CYAN);
        push(st_blue, s0, bddfalse, bddfalse);
        if (dfs_blue())
          return new emptiness_check_result;
        return 0;
      }

      virtual std::ostream& print_stats(std::ostream &os) const
      {
        os << nbn << " distinct nodes visited" << std::endl;
        os << nbt << " transitions explored" << std::endl;
        os << mdp << " nodes for the maximal stack depth" << std::endl;
        return os;
      }

    private:
      /// \brief counters for statistics (number of distinct nodes, of
      /// transitions and maximal stacks size.
      int nbn, nbt, mdp, sts;

      struct stack_item
      {
        stack_item(const state* n, tgba_succ_iterator* i, bdd l, bdd a)
          : s(n), it(i), label(l), acc(a) {};
        /// The visited state.
        const state* s;
        /// Design the next successor of \a s which has to be visited.
        tgba_succ_iterator* it;
        /// The label of the transition traversed to reach \a s
        /// (false for the first one).
        bdd label;
        /// The acceptance set of the transition traversed to reach \a s
        /// (false for the first one).
        bdd acc;
      };

      typedef std::list<stack_item> stack_type;

      void push(stack_type& st, const state* s,
                        const bdd& label, const bdd& acc)
      {
        ++sts;
        if (sts>mdp)
          mdp = sts;
        tgba_succ_iterator* i = a->succ_iter(s);
        i->first();
        st.push_front(stack_item(s, i, label, acc));
      }

      /// \brief Stack of the blue dfs.
      stack_type st_blue;

      /// \brief Stack of the red dfs.
      stack_type st_red;

      /// \brief Map where each visited state is colored
      /// by the last dfs visiting it.
      heap h;

      /// The automata to check.
      const tgba* a;

      /// The unique accepting condition of the automaton \a a.
      bdd all_acc;

      bool dfs_blue()
      {
        while (!st_blue.empty())
          {
            stack_item& f = st_blue.front();
#ifdef TRACE
            std::cout << "DFS_BLUE treats: "
                      << a->format_state(f.s) << std::endl;
#endif
            if (!f.it->done())
              {
                ++nbt;
                const state *s_prime = f.it->current_state();
                bdd label = f.it->current_condition();
                bdd acc = f.it->current_acceptance_conditions();
#ifdef TRACE
                std::cout << "  Visit the successor: "
                          << a->format_state(s_prime) << std::endl;
#endif
                f.it->next();
                typename heap::color_ref c_prime = h.get_color_ref(s_prime);
                if (c_prime.is_white())
                // Go down the edge (f.s, <label, acc>, s_prime)
                  {
                    ++nbn;
#ifdef TRACE
                    std::cout << "  It is white, go down" << std::endl;
#endif
                    h.add_new_state(s_prime, CYAN);
                    push(st_blue, s_prime, label, acc);
                  }
                else
                  {
                    typename heap::color_ref c = h.get_color_ref(f.s);
                    assert(!c.is_white());
                    if (c_prime.get_color() == CYAN &&
                      (c.get_acc() | acc | c_prime.get_acc()) == all_acc)
                      {
#ifdef TRACE
                          std::cout << "  It is cyan and acceptance condition "
                                    << "is reached, report cycle" << std::endl;
#endif
                        c_prime.cumulate_acc(c.get_acc() | acc);
                        push(st_red, s_prime, label, acc);
                        return true;
                      }
                    else // Backtrack the edge (f.s, <label, acc>, s_prime)
                      {
#ifdef TRACE
                          std::cout << "  It is cyan or blue, pop it"
                                    << std::endl;
#endif
                          h.pop_notify(s_prime);
                      }
                  }
              }
            else
            // Backtrack the edge
            //        (predecessor of f.s in st_blue, <f.label, f.acc>, f.s)
              {
#ifdef TRACE
                std::cout << "  All the successors have been visited"
                          << ", rescan this successors"
                          << std::endl;
#endif
                typename heap::color_ref c = h.get_color_ref(f.s);
                assert(!c.is_white());
                tgba_succ_iterator* i = a->succ_iter(f.s);
                for (i->first(); !i->done(); i->next())
                  {
                    ++nbt;
                    const state *s_prime = i->current_state();
                    bdd label = i->current_condition();
                    bdd acc = i->current_acceptance_conditions();
                    typename heap::color_ref c_prime = h.get_color_ref(s_prime);
                    assert(!c_prime.is_white());
                    bdd acu = acc | c.get_acc();
#ifdef TRACE
                    std::cout << "DFS_BLUE rescanning the arc from "
                              << a->format_state(f.s) << "  to "
                              << a->format_state(s_prime) << std::endl;
#endif
                    if ((c_prime.get_acc() & acu) != acu)
                      {
#ifdef TRACE
                        std::cout << "  ";
                        bdd_print_acc(std::cout, a->get_dict(), acu);
                        std::cout << "  is not included in ";
                        bdd_print_acc(std::cout, a->get_dict(),
                                                        c_prime.get_acc());
                        std::cout << std::endl;
                        std::cout << "  Start a red dfs from "
                                  << a->format_state(s_prime)
                                  << "  propagating: ";
                        bdd_print_acc(std::cout, a->get_dict(), acu);
                        std::cout << std::endl;
#endif
                        c_prime.cumulate_acc(acu);
                        push(st_red, s_prime, label, acc);
                        if (dfs_red(acu))
                          {
                            delete i;
                            return true;
                          }
                     }
                  }
                delete i;
                c.set_color(BLUE);
                delete f.it;
                --sts;
                h.pop_notify(f.s);
                st_blue.pop_front();
              }
          }
        return false;
      }

      bool dfs_red(const bdd& acu)
      {
        assert(!st_red.empty());

        while (!st_red.empty())
          {
            stack_item& f = st_red.front();
#ifdef TRACE
            std::cout << "DFS_RED treats: "
                      << a->format_state(f.s) << std::endl;
#endif
            if (!f.it->done()) // Go down
              {
                ++nbt;
                const state *s_prime = f.it->current_state();
                bdd label = f.it->current_condition();
                bdd acc = f.it->current_acceptance_conditions();
#ifdef TRACE
                std::cout << "  Visit the successor: "
                          << a->format_state(s_prime) << std::endl;
#endif
                f.it->next();
                typename heap::color_ref c_prime = h.get_color_ref(s_prime);
                if (!c_prime.is_white())
                  {
                    if (c_prime.get_color() == CYAN &&
                        (c_prime.get_acc() | acu) == all_acc)
                      {
#ifdef TRACE
                        std::cout << "  It is cyan and acceptance condition "
                                  << "is reached, report cycle" << std::endl;
#endif
                        c_prime.cumulate_acc(acu);
                        push(st_red, s_prime, label, acc);
                        return true;
                      }
                    else if ((c_prime.get_acc() & acu) != acu)
                      {
#ifdef TRACE
                        std::cout << "  It is cyan or blue and propagation "
                                  << "is needed, go down"
                                  << std::endl;
#endif
                        c_prime.cumulate_acc(acu);
                        push(st_red, s_prime, label, acc);
                      }
                    else
                      {
#ifdef TRACE
                        std::cout << "  It is cyan or blue and no propagation "
                                  << "is needed , pop it" << std::endl;
#endif
                        h.pop_notify(s_prime);
                      }
                  }
                else
                  {
#ifdef TRACE
                    std::cout << "  It is white, pop it" << std::endl;
#endif
                    delete s_prime;
                  }
              }
            else // Backtrack
              {
#ifdef TRACE
                std::cout << "  All the successors have been visited"
                          << ", pop it" << std::endl;
#endif
                --sts;
                h.pop_notify(f.s);
                delete f.it;
                st_red.pop_front();
              }
          }
        return false;
      }

    };

    class explicit_tau03_opt_search_heap
    {
    public:
      class color_ref
      {
      public:
        color_ref(color* c, bdd* a) :p(c), acc(a)
          {
          }
        color get_color() const
          {
            return *p;
          }
        void set_color(color c)
          {
            assert(!is_white());
            *p = c;
          }
        const bdd& get_acc() const
          {
            assert(!is_white());
            return *acc;
          }
        void cumulate_acc(const bdd& a)
          {
            assert(!is_white());
            *acc |= a;
          }
        bool is_white() const
          {
            return p == 0;
          }
      private:
        color *p;
        bdd* acc;
      };

      explicit_tau03_opt_search_heap(size_t)
        {
        }

      ~explicit_tau03_opt_search_heap()
        {
          hash_type::const_iterator s = h.begin();
          while (s != h.end())
            {
              // Advance the iterator before deleting the "key" pointer.
              const state* ptr = s->first;
              ++s;
              delete ptr;
            }
        }

      color_ref get_color_ref(const state*& s)
        {
          hash_type::iterator it = h.find(s);
          if (it==h.end())
            return color_ref(0, 0);
          if (s!=it->first)
            {
              delete s;
              s = it->first;
            }
          return color_ref(&(it->second.first), &(it->second.second));
        }

      void add_new_state(const state* s, color c)
        {
          assert(h.find(s)==h.end());
          h.insert(std::make_pair(s, std::make_pair(c, bddfalse)));
        }

      void pop_notify(const state*)
        {
        }

    private:

      typedef Sgi::hash_map<const state*, std::pair<color, bdd>,
                state_ptr_hash, state_ptr_equal> hash_type;
      hash_type h;
    };

  } // anonymous

  emptiness_check* explicit_tau03_opt_search(const tgba *a)
  {
    return new tau03_opt_search<explicit_tau03_opt_search_heap>(a, 0);
  }

}
