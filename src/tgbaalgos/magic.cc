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

#include <cstring>
#include <iostream>
#include "misc/hash.hh"
#include <list>
#include <iterator>
#include <cassert>
#include "magic.hh"

namespace spot
{
  namespace
  {
    enum color {WHITE, BLUE, RED};

    /// \brief Emptiness checker on spot::tgba automata having at most one
    /// accepting condition (i.e. a TBA).
    template <typename heap>
    class magic_search : public emptiness_check
    {
    public:
      /// \brief Initialize the Magic Search algorithm on the automaton \a a
      ///
      /// \pre The automaton \a a must have at most one accepting
      /// condition (i.e. it is a TBA).
      magic_search(const tgba *a, size_t size)
        : h(size), a(a), all_cond(a->all_acceptance_conditions())
      {
        assert(a->number_of_acceptance_conditions() <= 1);
      }

      virtual ~magic_search()
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
        nbn = nbt = 0;
        sts = mdp = st_blue.size() + st_red.size();
        if (st_red.empty())
          {
            assert(st_blue.empty());
            const state* s0 = a->get_init_state();
            ++nbn;
            h.add_new_state(s0, BLUE);
            push(st_blue, s0, bddfalse, bddfalse);
            if (dfs_blue())
              return new result(*this);
          }
        else
          {
            h.pop_notify(st_red.front().s);
            delete st_red.front().it;
            st_red.pop_front();
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
        os << nbn << " distinct nodes visited" << std::endl;
        os << nbt << " transitions explored" << std::endl;
        os << mdp << " nodes for the maximal stack depth" << std::endl;
        if (!st_red.empty())
          {
            assert(!st_blue.empty());
            os << st_blue.size() + st_red.size() - 1
               << " nodes for the counter example" << std::endl;
          }
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
        /// The label of the transition followed to reach \a s
        /// (false for the first one).
        bdd label;
        /// The acc set of the transition followed to reach \a s
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

      /// State targeted by the red dfs.
      const state* target;

      /// The automata to check.
      const tgba* a;

      /// The unique accepting condition of the automaton \a a.
      bdd all_cond;

      bool dfs_blue()
      {
        while (!st_blue.empty())
          {
            stack_item& f = st_blue.front();
            if (!f.it->done())
              {
                ++nbt;
                const state *s_prime = f.it->current_state();
                bdd label = f.it->current_condition();
                bdd acc = f.it->current_acceptance_conditions();
                f.it->next();
                typename heap::color_ref c = h.get_color_ref(s_prime);
                if (c.is_white())
                // Go down the edge (f.s, <label, acc>, s_prime)
                  {
                    ++nbn;
                    h.add_new_state(s_prime, BLUE);
                    push(st_blue, s_prime, label, acc);
                  }
                else // Backtrack the edge (f.s, <label, acc>, s_prime)
                  {
                    if (c.get() != RED && acc == all_cond)
                    // the test 'c.get() != RED' is added to limit
                    // the number of runs reported by successive
                    // calls to the check method. Without this
                    // functionnality, the test can be ommited.
                      {
                        target = f.s;
                        c.set(RED);
                        push(st_red, s_prime, label, acc);
                        if (dfs_red())
                          return true;
                      }
                    else
                      h.pop_notify(s_prime);
                  }
              }
            else
            // Backtrack the edge
            //        (predecessor of f.s in st_blue, <f.label, f.acc>, f.s)
              {
                --sts;
                stack_item f_dest(f);
                delete f.it;
                st_blue.pop_front();
                typename heap::color_ref c = h.get_color_ref(f_dest.s);
                assert(!c.is_white());
                if (c.get() != RED && f_dest.acc == all_cond
                                    && !st_blue.empty())
                // the test 'c.get() != RED' is added to limit
                // the number of runs reported by successive
                // calls to the check method. Without this
                // functionnality, the test can be ommited.
                  {
                    target = st_blue.front().s;
                    c.set(RED);
                    push(st_red, f_dest.s, f_dest.label, f_dest.acc);
                    if (dfs_red())
                      return true;
                  }
                else
                  h.pop_notify(f_dest.s);
              }
          }
        return false;
      }

      bool dfs_red()
      {
        assert(!st_red.empty());
        if (target->compare(st_red.front().s) == 0)
          return true;

        while (!st_red.empty())
          {
            stack_item& f = st_red.front();
            if (!f.it->done()) // Go down
              {
                ++nbt;
                const state *s_prime = f.it->current_state();
                bdd label = f.it->current_condition();
                bdd acc = f.it->current_acceptance_conditions();
                f.it->next();
                typename heap::color_ref c = h.get_color_ref(s_prime);
                if (c.is_white())
                // Notice that this case is taken into account only to  
                // support successive calls to the check method. Without
                // this functionnality, one can check assert(c.is_white()).
                // Go down the edge (f.s, <label, acc>, s_prime)
                  {
                    ++nbn;
                    h.add_new_state(s_prime, RED);
                    push(st_red, s_prime, label, acc);
                  }
                else // Go down the edge (f.s, <label, acc>, s_prime)
                  {
                    if (c.get() != RED)
                      {
                        c.set(RED);
                        push(st_red, s_prime, label, acc);
                        if (target->compare(s_prime) == 0)
                          return true;
                      }
                    else
                      h.pop_notify(s_prime);
                  }
              }
            else // Backtrack
              {
                --sts;
                h.pop_notify(f.s);
                delete f.it;
                st_red.pop_front();
              }
          }
        return false;
      }

      class result: public emptiness_check_result
      {
      public:
        result(magic_search& ms)
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

          l = &run->prefix;

          i = ms_.st_blue.rbegin();
          end = ms_.st_blue.rend(); --end;
          j = i; ++j;
          for (; i != end; ++i, ++j)
            {
              tgba_run::step s = { i->s->clone(), j->label, j->acc };
              l->push_back(s);
            }

          l = &run->cycle;

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
        magic_search& ms_;
      };

    };

    class explicit_magic_search_heap
    {
    public:
      class color_ref
      {
      public:
        color_ref(color* c) :p(c)
          {
          }
        color get() const
          {
            return *p;
          }
        void set(color c)
          {
            assert(!is_white());
            *p=c;
          }
        bool is_white() const
          {
            return p==0;
          }
      private:
        color *p;
      };

      explicit_magic_search_heap(size_t)
        {
        }

      ~explicit_magic_search_heap()
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
            return color_ref(0);
          if (s!=it->first)
            {
              delete s;
              s = it->first;
            }
          return color_ref(&(it->second));
        }

      void add_new_state(const state* s, color c)
        {
          assert(h.find(s)==h.end());
          h.insert(std::make_pair(s, c));
        }

      void pop_notify(const state*)
        {
        }

    private:

      typedef Sgi::hash_map<const state*, color,
                state_ptr_hash, state_ptr_equal> hash_type;
      hash_type h;
    };

    class bsh_magic_search_heap
    {
    public:
      class color_ref
      {
      public:
        color_ref(unsigned char *b, unsigned char o): base(b), offset(o*2)
          {
          }
        color get() const
          {
            return color(((*base) >> offset) & 3U);
          }
        void set(color c)
          {
            *base =  (*base & ~(3U << offset)) | (c << offset);
          }
        bool is_white() const
          {
            return get()==WHITE;
          }
      private:
        unsigned char *base;
        unsigned char offset;
      };

      bsh_magic_search_heap(size_t s)
        {
          size = s;
          h = new unsigned char[size];
          memset(h, WHITE, size);
        }

      ~bsh_magic_search_heap()
        {
          delete[] h;
        }

      color_ref get_color_ref(const state*& s)
        {
          size_t ha = s->hash();
          return color_ref(&(h[ha%size]), ha%4);
        }

      void add_new_state(const state* s, color c)
        {
          color_ref cr(get_color_ref(s));
          assert(cr.is_white());
          cr.set(c);
        }

      void pop_notify(const state* s)
        {
          delete s;
        }

    private:
      size_t size;
      unsigned char* h;
    };

  } // anonymous

  emptiness_check* explicit_magic_search(const tgba *a)
  {
    return new magic_search<explicit_magic_search_heap>(a, 0);
  }

  emptiness_check* bit_state_hashing_magic_search(
                            const tgba *a, size_t size)
  {
    return new magic_search<bsh_magic_search_heap>(a, size);
  }

}
