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

/// FIXME:
///
/// * Add the computation of a counter example if detected.
///
/// * Add some heuristics on the order of visit of the successors in the blue
///   dfs:
///   - favorize the arcs conducting to the blue stack (the states of color
///     cyan)
///   - in this category, favorize the arcs labelled
///   - for the remaining ones, favorize the arcs labelled by the greatest
///     number of new acceptance conditions (notice that this number may evolve
///     after the visit of previous successors).
///
/// * Add a bit-state hashing version.
///
/// * Is it possible to reduce the tgba on-the-fly during the product: only the
///   acceptance conditions are pertinent...

//#define TRACE

#ifdef TRACE
#include <iostream>
#include "tgba/bddprint.hh"
#endif

#include <cassert>
#include <list>
#include "misc/hash.hh"
#include "tgba/tgba.hh"
#include "emptiness.hh"
#include "emptiness_stats.hh"
#include "tau03opt.hh"
#include "weight.hh"

namespace spot
{
  namespace
  {
    enum color {WHITE, CYAN, BLUE};

    /// \brief Emptiness checker on spot::tgba automata having at most one
    /// accepting condition (i.e. a TBA).
    template <typename heap>
    class tau03_opt_search : public emptiness_check, public ec_statistics
    {
    public:
      /// \brief Initialize the search algorithm on the automaton \a a
      tau03_opt_search(const tgba *a, size_t size)
        : emptiness_check(a),
          current_weight(a->neg_acceptance_conditions()),
          h(size),
          all_acc(a->all_acceptance_conditions())
      {
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
        if (!st_blue.empty())
            return 0;
        assert(st_red.empty());
        const state* s0 = a_->get_init_state();
        inc_states();
        h.add_new_state(s0, CYAN, current_weight);
        push(st_blue, s0, bddfalse, bddfalse);
        if (dfs_blue())
          return new emptiness_check_result(a_);
        return 0;
      }

      virtual std::ostream& print_stats(std::ostream &os) const
      {
        os << states() << " distinct nodes visited" << std::endl;
        os << transitions() << " transitions explored" << std::endl;
        os << max_depth() << " nodes for the maximal stack depth" << std::endl;
        return os;
      }

    private:
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
        inc_depth();
        tgba_succ_iterator* i = a_->succ_iter(s);
        i->first();
        st.push_front(stack_item(s, i, label, acc));
      }

      void pop(stack_type& st)
      {
        dec_depth();
        delete st.front().it;
        st.pop_front();
      }

      /// \brief weight of the state on top of the blue stack.
      weight current_weight;

      /// \brief Stack of the blue dfs.
      stack_type st_blue;

      /// \brief Stack of the red dfs.
      stack_type st_red;

      /// \brief Map where each visited state is colored
      /// by the last dfs visiting it.
      heap h;

      /// The unique accepting condition of the automaton \a a.
      bdd all_acc;

      bool dfs_blue()
      {
        while (!st_blue.empty())
          {
            stack_item& f = st_blue.front();
#           ifdef TRACE
            std::cout << "DFS_BLUE treats: "
                      << a_->format_state(f.s) << std::endl;
#           endif
            if (!f.it->done())
              {
                const state *s_prime = f.it->current_state();
#               ifdef TRACE
                std::cout << "  Visit the successor: "
                          << a_->format_state(s_prime) << std::endl;
#               endif
                bdd label = f.it->current_condition();
                bdd acc = f.it->current_acceptance_conditions();
                // Go down the edge (f.s, <label, acc>, s_prime)
                f.it->next();
                inc_transitions();
                typename heap::color_ref c_prime = h.get_color_ref(s_prime);
                if (c_prime.is_white())
                  {
#                   ifdef TRACE
                    std::cout << "  It is white, go down" << std::endl;
#                   endif
                    current_weight += acc;
                    inc_states();
                    h.add_new_state(s_prime, CYAN, current_weight);
                    push(st_blue, s_prime, label, acc);
                  }
                else
                  {
                    typename heap::color_ref c = h.get_color_ref(f.s);
                    assert(!c.is_white());
                    if (c_prime.get_color() == CYAN &&
                      ((current_weight - c_prime.get_weight()) |
                          c.get_acc() | acc | c_prime.get_acc()) == all_acc)
                      {
#                       ifdef TRACE
                        std::cout << "  It is cyan and acceptance condition "
                                  << "is reached, report cycle" << std::endl;
#                       endif
                        c_prime.cumulate_acc(c.get_acc() | acc);
                        push(st_red, s_prime, label, acc);
                        return true;
                      }
                    else
                      {
#                       ifdef TRACE
                        std::cout << "  It is cyan or blue and";
#                       endif
                        bdd acu = acc | c.get_acc();
                        if ((c_prime.get_acc() & acu) != acu)
                          {
#                           ifdef TRACE
                            bdd_print_acc(std::cout, a_->get_dict(), acu);
                            std::cout << "  is not included in ";
                            bdd_print_acc(std::cout, a_->get_dict(),
					  c_prime.get_acc());
                            std::cout << ", start a red dfs propagating: ";
                            bdd_print_acc(std::cout, a_->get_dict(), acu);
                            std::cout << std::endl;
#                           endif
                            c_prime.cumulate_acc(acu);
                            push(st_red, s_prime, label, acc);
                            if (dfs_red(acu))
                              return true;
                          }
                        else
                          {
#                           ifdef TRACE
                            std::cout << " no propagation is needed, pop it."
                                      << std::endl;
#                           endif
                            h.pop_notify(s_prime);
                          }
                      }
                  }
              }
            else
            // Backtrack the edge
            //        (predecessor of f.s in st_blue, <f.label, f.acc>, f.s)
              {
#               ifdef TRACE
                std::cout << "  All the successors have been visited"
                          << std::endl;
#               endif
                stack_item f_dest(f);
                pop(st_blue);
                current_weight -= f_dest.acc;
                typename heap::color_ref c_prime = h.get_color_ref(f_dest.s);
                assert(!c_prime.is_white());
                c_prime.set_color(BLUE);
                if (!st_blue.empty())
                  {
                    typename heap::color_ref c =
                                          h.get_color_ref(st_blue.front().s);
                    assert(!c.is_white());
                    bdd acu = f_dest.acc | c.get_acc();
                    if ((c_prime.get_acc() & acu) != acu)
                      {
#                       ifdef TRACE
                        std::cout << "  The arc from "
                                  << a_->format_state(st_blue.front().s)
                                  << " to the current state implies to "
                                  << " start a red dfs propagating ";
                                  bdd_print_acc(std::cout, a_->get_dict(), acu);
                        std::cout << std::endl;
#                       endif
                        c_prime.cumulate_acc(acu);
                        push(st_red, f_dest.s, f_dest.label, f_dest.acc);
                        if (dfs_red(acu))
                            return true;
                      }
                    else
                      {
#                       ifdef TRACE
                        std::cout << "  Pop it" << std::endl;
#                       endif
                        h.pop_notify(f_dest.s);
                      }
                  }
                else
                  {
#                   ifdef TRACE
                    std::cout << "  Pop it" << std::endl;
#                   endif
                    h.pop_notify(f_dest.s);
                  }
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
#           ifdef TRACE
            std::cout << "DFS_RED treats: "
                      << a_->format_state(f.s) << std::endl;
#           endif
            if (!f.it->done())
              {
                const state *s_prime = f.it->current_state();
#               ifdef TRACE
                std::cout << "  Visit the successor: "
                          << a_->format_state(s_prime) << std::endl;
#               endif
                bdd label = f.it->current_condition();
                bdd acc = f.it->current_acceptance_conditions();
                // Go down the edge (f.s, <label, acc>, s_prime)
                f.it->next();
                inc_transitions();
                typename heap::color_ref c_prime = h.get_color_ref(s_prime);
                if (c_prime.is_white())
                  {
#                   ifdef TRACE
                    std::cout << "  It is white, pop it" << std::endl;
#                   endif
                    delete s_prime;
                  }
                 else if (c_prime.get_color() == CYAN &&
                     ((current_weight - c_prime.get_weight()) |
                                c_prime.get_acc() | acu) == all_acc)
                  {
#                   ifdef TRACE
                    std::cout << "  It is cyan and acceptance condition "
                              << "is reached, report cycle" << std::endl;
#                   endif
                    c_prime.cumulate_acc(acu);
                    push(st_red, s_prime, label, acc);
                    return true;
                  }
                else if ((c_prime.get_acc() & acu) != acu)
                  {
#                   ifdef TRACE
                    std::cout << "  It is cyan or blue and propagation "
                              << "is needed, go down"
                              << std::endl;
#                   endif
                    c_prime.cumulate_acc(acu);
                    push(st_red, s_prime, label, acc);
                  }
                else
                  {
#                   ifdef TRACE
                    std::cout << "  It is cyan or blue and no propagation "
                              << "is needed , pop it" << std::endl;
#                   endif
                    h.pop_notify(s_prime);
                  }
              }
            else // Backtrack
              {
#               ifdef TRACE
                std::cout << "  All the successors have been visited, pop it"
                          << std::endl;
#               endif
                h.pop_notify(f.s);
                pop(st_red);
              }
          }
        return false;
      }

    };

    class explicit_tau03_opt_search_heap
    {
      typedef Sgi::hash_map<const state*, std::pair<weight, bdd>,
                state_ptr_hash, state_ptr_equal> hcyan_type;
      typedef Sgi::hash_map<const state*, std::pair<color, bdd>,
                state_ptr_hash, state_ptr_equal> hash_type;
    public:
      class color_ref
      {
      public:
        color_ref(hash_type* h, hcyan_type* hc, const state* s,
            const weight* w, bdd* a)
          : is_cyan(true), w(w), ph(h), phc(hc), ps(s), acc(a)
          {
          }
        color_ref(color* c, bdd* a)
          : is_cyan(false), pc(c), acc(a)
          {
          }
        color get_color() const
          {
            if (is_cyan)
              return CYAN;
            return *pc;
          }
        const weight& get_weight() const
          {
            assert(is_cyan);
            return *w;
          }
        void set_color(color c)
          {
            assert(!is_white());
            if (is_cyan)
              {
                assert(c != CYAN);
                std::pair<hash_type::iterator, bool> p;
                p = ph->insert(std::make_pair(ps, std::make_pair(c, *acc)));
                assert(p.second);
                acc = &(p.first->second.second);
                int i = phc->erase(ps);
                assert(i==1);
                (void)i;
              }
            else
              {
                *pc=c;
              }
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
            return !is_cyan && pc == 0;
          }
      private:
        bool is_cyan;
        const weight* w; // point to the weight of a state in hcyan
        hash_type* ph; //point to the main hash table
        hcyan_type* phc; // point to the hash table hcyan
        const state* ps; // point to the state in hcyan
        color *pc; // point to the color of a state stored in main hash table
        bdd* acc; // point to the acc set of a state stored in main hash table
                  // or hcyan
      };

      explicit_tau03_opt_search_heap(size_t)
        {
        }

      ~explicit_tau03_opt_search_heap()
        {
          hcyan_type::const_iterator sc = hc.begin();
          while (sc != hc.end())
            {
              const state* ptr = sc->first;
              ++sc;
              delete ptr;
            }
          hash_type::const_iterator s = h.begin();
          while (s != h.end())
            {
              const state* ptr = s->first;
              ++s;
              delete ptr;
            }
        }

      color_ref get_color_ref(const state*& s)
        {
          hcyan_type::iterator ic = hc.find(s);
          if (ic==hc.end())
            {
              hash_type::iterator it = h.find(s);
              if (it==h.end())
                // white state
                return color_ref(0, 0);
              if (s!=it->first)
                {
                  delete s;
                  s = it->first;
                }
              // blue or red state
              return color_ref(&(it->second.first), &(it->second.second));
            }
          if (s!=ic->first)
            {
              delete s;
              s = ic->first;
            }
          // cyan state
          return color_ref(&h, &hc, ic->first,
                              &(ic->second.first), &(ic->second.second));
        }

      void add_new_state(const state* s, color c)
        {
          assert(hc.find(s)==hc.end() && h.find(s)==h.end());
          assert(c != CYAN);
          h.insert(std::make_pair(s, std::make_pair(c, bddfalse)));
        }

      void add_new_state(const state* s, color c, const weight& w)
        {
          assert(hc.find(s)==hc.end() && h.find(s)==h.end());
          assert(c == CYAN);
          (void)c;
          hc.insert(std::make_pair(s, std::make_pair(w, bddfalse)));
        }

      void pop_notify(const state*)
        {
        }

    private:

      // associate to each blue and red state its color and its acceptance set
      hash_type h;
      // associate to each cyan state its weight and its acceptance set
      hcyan_type hc;
    };

  } // anonymous

  emptiness_check* explicit_tau03_opt_search(const tgba *a)
  {
    return new tau03_opt_search<explicit_tau03_opt_search_heap>(a, 0);
  }

}
