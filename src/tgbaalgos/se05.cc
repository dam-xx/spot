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

//#define TRACE

#ifdef TRACE
#include <iostream>
#endif

#include <cassert>
#include <list>
#include "misc/hash.hh"
#include "tgba/tgba.hh"
#include "emptiness.hh"
#include "emptiness_stats.hh"
#include "se05.hh"

/// FIXME: make compiling depedent the taking into account of weights.

namespace spot
{
  namespace
  {
    enum color {WHITE, CYAN, BLUE, RED};

    /// \brief Emptiness checker on spot::tgba automata having at most one
    /// accepting condition (i.e. a TBA).
    template <typename heap>
    class se05_search : public emptiness_check, public ec_statistics
    {
    public:
      /// \brief Initialize the Magic Search algorithm on the automaton \a a
      ///
      /// \pre The automaton \a a must have at most one accepting
      /// condition (i.e. it is a TBA).
      se05_search(const tgba *a, size_t size)
        : ec_statistics(),
          current_weight(0),
          h(size),
          a(a),
          all_cond(a->all_acceptance_conditions())
      {
        assert(a->number_of_acceptance_conditions() <= 1);
      }

      virtual ~se05_search()
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
      /// \return non null pointer iff the algorithm has found a
      /// new accepting path.
      ///
      /// check() can be called several times (until it returns a null
      /// pointer) to enumerate all the visited accepting paths. The method
      /// visits only a finite set of accepting paths.
      virtual emptiness_check_result* check()
      {
        if (st_red.empty())
          {
            assert(st_blue.empty());
            const state* s0 = a->get_init_state();
            inc_states();
            h.add_new_state(s0, CYAN, current_weight);
            push(st_blue, s0, bddfalse, bddfalse);
            if (dfs_blue())
              return new result(*this);
          }
        else
          {
            h.pop_notify(st_red.front().s);
            pop(st_red);
            if (!st_red.empty() && dfs_red())
              return new result(*this);
            else
              if (dfs_blue())
                return new result(*this);
          }
        return 0;
      }

      virtual std::ostream& print_stats(std::ostream &os) const
      {
        os << states() << " distinct nodes visited" << std::endl;
        os << transitions() << " transitions explored" << std::endl;
        os << max_depth() << " nodes for the maximal stack depth" << std::endl;
        if (!st_red.empty())
          {
            assert(!st_blue.empty());
            os << st_blue.size() + st_red.size() - 1
               << " nodes for the counter example" << std::endl;
          }
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
        tgba_succ_iterator* i = a->succ_iter(s);
        i->first();
        st.push_front(stack_item(s, i, label, acc));
      }

      void pop(stack_type& st)
      {
        dec_depth();
        delete st.front().it;
        st.pop_front();
      }

      /// \brief number of visited accepting arcs
      /// in the blue stack.
      int current_weight;

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
      bdd all_cond;

      bool dfs_blue()
      {
        while (!st_blue.empty())
          {
            stack_item& f = st_blue.front();
#           ifdef TRACE
            std::cout << "DFS_BLUE treats: "
                      << a->format_state(f.s) << std::endl;
#           endif
            if (!f.it->done())
              {
                const state *s_prime = f.it->current_state();
#               ifdef TRACE
                std::cout << "  Visit the successor: "
                          << a->format_state(s_prime) << std::endl;
#               endif
                bdd label = f.it->current_condition();
                bdd acc = f.it->current_acceptance_conditions();
                // Go down the edge (f.s, <label, acc>, s_prime)
                f.it->next();
                inc_transitions();
                typename heap::color_ref c = h.get_color_ref(s_prime);
                if (c.is_white())
                  {
#                   ifdef TRACE
                    std::cout << "  It is white, go down" << std::endl;
#                   endif
                    if (acc == all_cond)
                      ++current_weight;
                    inc_states();
                    h.add_new_state(s_prime, CYAN, current_weight);
                    push(st_blue, s_prime, label, acc);
                  }
                else if (c.get_color() == CYAN && (acc == all_cond ||
             (f.s->compare(s_prime) != 0 && f.acc == all_cond) // option SE05
//                            current_weight > c.get_weight()  // option WEIGHT
                                              /**/))
// For Alexandre: combat style.test! ----------^
                  {
#                   ifdef TRACE
                    std::cout << "  It is cyan and acceptance condition "
                              << "is reached, report cycle" << std::endl;
#                   endif
                    c.set_color(RED);
                    push(st_red, s_prime, label, acc);
                    return true;
                  }
                else if (acc == all_cond && c.get_color() != RED)
                  {
                    // the test 'c.get_color() != RED' is added to limit
                    // the number of runs reported by successive
                    // calls to the check method. Without this
                    // functionnality, the test can be ommited.
#                   ifdef TRACE
                    std::cout << "  It is cyan or blue and the arc is "
                              << "accepting, start a red dfs" << std::endl;
#                   endif
                    c.set_color(RED);
                    push(st_red, s_prime, label, acc);
                    if (dfs_red())
                      return true;
                  }
                else
                  {
#                   ifdef TRACE
                    std::cout << "  It is cyan, blue or red, pop it"
                              << std::endl;
#                   endif
                    h.pop_notify(s_prime);
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
                if (f_dest.acc == all_cond)
                  --current_weight;
                typename heap::color_ref c = h.get_color_ref(f_dest.s);
                assert(!c.is_white());
                if (!st_blue.empty() &&
                          f_dest.acc == all_cond && c.get_color() != RED)
                  {
                    // the test 'c.get_color() != RED' is added to limit
                    // the number of runs reported by successive
                    // calls to the check method. Without this
                    // functionnality, the test can be ommited.
#                   ifdef TRACE
                    std::cout << "  The arc from "
                              << a->format_state(st_blue.front().s)
                              << " to the current state is accepting, start a "
                              << "red dfs" << std::endl;
#                   endif
                    c.set_color(RED);
                    push(st_red, f_dest.s, f_dest.label, f_dest.acc);
                    if (dfs_red())
                      return true;
                  }
                else
                  {
#                   ifdef TRACE
                    std::cout << "  Pop it" << std::endl;
#                   endif
                    c.set_color(BLUE);
                    h.pop_notify(f_dest.s);
                  }
              }
          }
        return false;
      }

      bool dfs_red()
      {
        assert(!st_red.empty());

        while (!st_red.empty())
          {
            stack_item& f = st_red.front();
#           ifdef TRACE
            std::cout << "DFS_RED treats: "
                      << a->format_state(f.s) << std::endl;
#           endif
            if (!f.it->done())
              {
                const state *s_prime = f.it->current_state();
#               ifdef TRACE
                std::cout << "  Visit the successor: "
                          << a->format_state(s_prime) << std::endl;
#               endif
                bdd label = f.it->current_condition();
                bdd acc = f.it->current_acceptance_conditions();
                // Go down the edge (f.s, <label, acc>, s_prime)
                f.it->next();
                inc_transitions();
                typename heap::color_ref c = h.get_color_ref(s_prime);
                if (c.is_white())
                  {
                    // For an explicit search, we can pose assert(!c.is_white())
                    // because to reach a white state, the red dfs must
                    // have crossed a cyan one (a state in the blue stack)
                    // implying the report of a cycle.
                    // However, with a bit-state hashing search and due to
                    // collision, this property does not hold.
#                   ifdef TRACE
                    std::cout << "  It is white (due to collision), pop it"
                              << std::endl;
#                   endif
                    delete s_prime;
                  }
                else if (c.get_color() == RED)
                  {
#                   ifdef TRACE
                    std::cout << "  It is red, pop it"
                              << std::endl;
#                   endif
                    h.pop_notify(s_prime);
                  }
                else if (c.get_color() == CYAN)
                  {
#                   ifdef TRACE
                    std::cout << "  It is cyan, report a cycle"
                              << std::endl;
#                   endif
                    c.set_color(RED);
                    push(st_red, s_prime, label, acc);
                    return true;
                  }
                else
                  {
#                   ifdef TRACE
                    std::cout << "  It is blue, go down" << std::endl;
#                   endif
                    c.set_color(RED);
                    push(st_red, s_prime, label, acc);
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

      class result: public emptiness_check_result
      {
      public:
        result(se05_search& ms)
          : ms_(ms)
        {
        }
        virtual tgba_run* accepting_run()
        {
          assert(!ms_.st_blue.empty());
          assert(!ms_.st_red.empty());

          tgba_run* run = new tgba_run;

          typename stack_type::const_reverse_iterator i, j, end;
          tgba_run::steps* l;

          const state* target = ms_.st_red.front().s;

          l = &run->prefix;

          i = ms_.st_blue.rbegin();
          end = ms_.st_blue.rend(); --end;
          j = i; ++j;
          for (; i != end; ++i, ++j)
            {
              if (l == &run->prefix && i->s->compare(target) == 0)
                l = &run->cycle;
              tgba_run::step s = { i->s->clone(), j->label, j->acc };
              l->push_back(s);
            }

          if (l == &run->prefix && i->s->compare(target) == 0)
            l = &run->cycle;
          assert(l == &run->cycle);

          j = ms_.st_red.rbegin();
          tgba_run::step s = { i->s->clone(), j->label, j->acc };
          l->push_back(s);

          i = j; ++j;
          end = ms_.st_red.rend(); --end;
          for (; i != end; ++i, ++j)
            {
              tgba_run::step s = { i->s->clone(), j->label, j->acc };
              l->push_back(s);
            }

          return run;
        }
      private:
        se05_search& ms_;
      };

    };

    class explicit_se05_search_heap
    {
      typedef Sgi::hash_map<const state*, int,
                state_ptr_hash, state_ptr_equal> hcyan_type;
      typedef Sgi::hash_map<const state*, color,
                state_ptr_hash, state_ptr_equal> hash_type;
    public:
      class color_ref
      {
      public:
        color_ref(hash_type* h, hcyan_type* hc, const state* s, int w)
          : is_cyan(true), weight(w), ph(h), phc(hc), ps(s), pc(0)
          {
          }
        color_ref(color* c)
          : is_cyan(false), weight(0), ph(0), phc(0), ps(0), pc(c)
          {
          }
        color get_color() const
          {
            if (is_cyan)
              return CYAN;
            return *pc;
          }
        int get_weight() const
          {
            assert(is_cyan);
            return weight;
          }
        void set_color(color c)
          {
            assert(!is_white());
            if (is_cyan)
              {
                assert(c != CYAN);
                int i = phc->erase(ps);
                assert(i==1);
                (void)i;
                ph->insert(std::make_pair(ps, c));
              }
            else
              {
                *pc=c;
              }
          }
        bool is_white() const
          {
            return !is_cyan && pc==0;
          }
      private:
        bool is_cyan;
        int weight; // weight of a cyan node
        hash_type* ph; //point to the main hash table
        hcyan_type* phc; // point to the hash table hcyan
        const state* ps; // point to the state in hcyan
        color *pc; // point to the color of a state stored in main hash table
      };

      explicit_se05_search_heap(size_t)
        {
        }

      ~explicit_se05_search_heap()
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
                return color_ref(0); // white state
              if (s!=it->first)
                {
                  delete s;
                  s = it->first;
                }
              return color_ref(&(it->second)); // blue or red state
            }
          if (s!=ic->first)
            {
              delete s;
              s = ic->first;
            }
          return color_ref(&h, &hc, ic->first, ic->second); // cyan state
        }

      void add_new_state(const state* s, color c, int w=-1)
        {
          assert(hc.find(s)==hc.end() && h.find(s)==h.end());
          assert(c!=CYAN || w>=0);
          if (c == CYAN)
            hc.insert(std::make_pair(s, w));
          else
            h.insert(std::make_pair(s, c));
        }

      void pop_notify(const state*)
        {
        }

    private:

      hash_type h; // associate to each blue and red state its color
      hcyan_type hc; // associate to each cyan state its weight
    };

    class bsh_se05_search_heap
    {
    private:
      typedef Sgi::hash_map<const state*, int,
                state_ptr_hash, state_ptr_equal> hcyan_type;
    public:
      class color_ref
      {
      public:
        color_ref(hcyan_type* h, const state* st, int w,
                    unsigned char *base, unsigned char offset)
          : is_cyan(true), weight(w), phc(h), ps(st), b(base), o(offset*2)
          {
          }
        color_ref(unsigned char *base, unsigned char offset)
          : is_cyan(false), weight(0), phc(0), ps(0), b(base), o(offset*2)
          {
          }
        color get_color() const
          {
            if (is_cyan)
              return CYAN;
            return color(((*b) >> o) & 3U);
          }
        int get_weight() const
          {
            assert(is_cyan);
            return weight;
          }
        void set_color(color c)
          {
            if (is_cyan && c!=CYAN)
              {
                int i = phc->erase(ps);
                assert(i==1);
                (void)i;
              }
            *b =  (*b & ~(3U << o)) | (c << o);
          }
        bool is_white() const
          {
            return get_color()==WHITE;
          }
      private:
        bool is_cyan;
        int weight;
        hcyan_type* phc;
        const state* ps;
        unsigned char *b;
        unsigned char o;
      };

      bsh_se05_search_heap(size_t s) : size(s)
        {
          h = new unsigned char[size];
          memset(h, WHITE, size);
        }

      ~bsh_se05_search_heap()
        {
          delete[] h;
        }

      color_ref get_color_ref(const state*& s)
        {
          size_t ha = s->hash();
          hcyan_type::iterator ic = hc.find(s);
          if (ic!=hc.end())
            return color_ref(&hc, ic->first, ic->second, &h[ha%size], ha%4);
          return color_ref(&h[ha%size], ha%4);
        }

      void add_new_state(const state* s, color c, int w=-1)
        {
          assert(c!=CYAN || w>=0);
          assert(get_color_ref(s).is_white());
          if (c==CYAN)
            hc.insert(std::make_pair(s, w));
          else
            {
              color_ref cr(get_color_ref(s));
              cr.set_color(c);
            }
        }

      void pop_notify(const state* s)
        {
          delete s;
        }

    private:
      size_t size;
      unsigned char* h;
      hcyan_type hc;
    };

  } // anonymous

  emptiness_check* explicit_se05_search(const tgba *a)
  {
    return new se05_search<explicit_se05_search_heap>(a, 0);
  }

  emptiness_check* bit_state_hashing_se05_search(const tgba *a, size_t size)
  {
    return new se05_search<bsh_se05_search_heap>(a, size);
  }

}
