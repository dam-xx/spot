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

#ifndef SPOT_TGBAALGOS_NDFS_RESULT_HH
# define SPOT_TGBAALGOS_NDFS_RESULT_HH

//#define TRACE

#include <iostream>
#ifdef TRACE
#define trace std::cerr
#else
#define trace while (0) std::cerr
#endif

#include <cassert>
#include <list>
#include "tgba/tgba.hh"
#include "emptiness.hh"
#include "emptiness_stats.hh"
#include "bfssteps.hh"
#include "misc/hash.hh"

/// FIXME:
/// * Add the necessary calls to pop_notify.

namespace spot
{
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

  template < typename ndfs_search, typename heap >
  class ndfs_result : public emptiness_check_result
  {
  public:
    ndfs_result(const ndfs_search& ms)
    : emptiness_check_result(ms.automaton()), ms_(ms), h_(ms_.get_heap())
      {
      }

    virtual ~ndfs_result()
      {
        while (!st1.empty())
          {
            delete st1.front().it;
            st1.pop_front();
          }
      }

    virtual tgba_run* accepting_run()
      {
        const stack_type& stb = ms_.get_st_blue();
        const stack_type& str = ms_.get_st_red();

        assert(!stb.empty());

        tgba_run* run = new tgba_run;

        const state* target = str.empty()?stb.front().s:str.front().s;
        bdd covered_acc = bddfalse;
        typename stack_type::const_reverse_iterator i, j;

        i = j = stb.rbegin(); ++j;
        for (; i->s->compare(target) != 0; ++i, ++j)
          {
            tgba_run::step s = { i->s->clone(), j->label, j->acc };
            run->prefix.push_back(s);
          }

        if (!str.empty())
          {
            typename stack_type::const_reverse_iterator end = stb.rend();
            for (; j != end; ++i, ++j)
              {
                covered_acc |= j->acc;
                tgba_run::step s = { i->s->clone(), j->label, j->acc };
                run->cycle.push_back(s);
              }

            j = str.rbegin();
            covered_acc |= j->acc;
            tgba_run::step s = { i->s->clone(), j->label, j->acc };
            run->cycle.push_back(s);

            i = j; ++j;
            end = str.rend();
            for (; j != end; ++i, ++j)
              {
                covered_acc |= j->acc;
                tgba_run::step s = { i->s->clone(), j->label, j->acc };
                run->cycle.push_back(s);
              }
          }

        if (a_->all_acceptance_conditions() != covered_acc)
          {
            // try if any to minimize the first loop in run->cycle ??
            // what transitions have to be preserved  (it depend on
            // the detection (in the blue or red dfs) ??
            tgba_succ_iterator* i = a_->succ_iter(target);
            i->first();
            st1.push_front(stack_item(target, i, bddfalse, bddfalse));
            bool b = dfs(target, run->cycle, covered_acc);
            assert(b);
            (void)b;
            while (!st1.empty())
              {
                delete st1.front().it;
                st1.pop_front();
              }
          }

        return run;
      }

  private:
    const ndfs_search& ms_;
    const heap& h_;
    stack_type st1;

    typedef Sgi::hash_set<const state*,
                          state_ptr_hash, state_ptr_equal> state_set;

    class shortest_path: public bfs_steps
    {
    public:
      shortest_path(const tgba* a, const state* t,
                  const state_set& d, const heap& h)
        : bfs_steps(a), target(t), dead(d), h(h)
      {
      }

      ~shortest_path()
      {
      }

      const state*
      search(const state* start, tgba_run::steps& l)
      {
        const state* s = filter(start);
        if (s)
          return this->bfs_steps::search(s, l);
        else
          return 0;
      }

      const state*
      filter(const state* s)
      {
        if (!h.has_been_visited(s))
          {
            delete s;
            return 0;
          }
        if (dead.find(s) != dead.end())
          return 0;
        seen.insert(s);
        return s;
      }

      const state_set&
      get_seen() const
      {
        return seen;
      }

      bool
      match(tgba_run::step&, const state* dest)
      {
        return target->compare(dest) == 0;
      }

    private:
      state_set seen;
      const state* target;
      const state_set& dead;
      const heap& h;
    };

    void complete_cycle(tgba_run::steps& cycle, bdd& covered_acc,
                  const tgba_run::step& start, tgba_run::steps& path,
                  const state_set& dead)
      {
        tgba_run::steps new_cycle;

        // find the minimal path between st1.back().s and st1.front().s
        if (st1.back().s->compare(st1.front().s)!=0)
          {
            shortest_path s(a_, st1.front().s, dead, h_);
            const state* res = s.search(st1.back().s, new_cycle);
            assert(res && res->compare(st1.front().s) == 0);
            (void)res;
            for (tgba_run::steps::const_iterator it = new_cycle.begin();
                                                  it != new_cycle.end(); ++it)
              covered_acc |= it->acc;
          }

        // traverse the accepting transition
        covered_acc |= start.acc;
        tgba_run::step s = { st1.front().s->clone(), start.label, start.acc };
        new_cycle.push_back(s);

        // follow the minimal path returning to st1.back().s
        for (tgba_run::steps::const_iterator it = path.begin();
                                                    it != path.end(); ++it)
          covered_acc |= it->acc;
        new_cycle.splice(new_cycle.end(), path);

        // concat this new loop to the existing ones
        cycle.splice(cycle.end(), new_cycle);
      }

    bool dfs(const state* target, tgba_run::steps& cycle, bdd& covered_acc)
      {
        state_set seen, dead;

        seen.insert(target);
        while (!st1.empty())
          {
            stack_item& f = st1.front();
            trace << "DFS1 treats: " << a_->format_state(f.s) << std::endl;
            if (!f.it->done())
              {
                const state *s_prime = f.it->current_state();
                trace << "  Visit the successor: "
                      << a_->format_state(s_prime) << std::endl;
                bdd label = f.it->current_condition();
                bdd acc = f.it->current_acceptance_conditions();
                f.it->next();
                if (h_.has_been_visited(s_prime))
                  {
                    if (dead.find(s_prime) != dead.end())
                      {
                        trace << "  it is dead, pop it" << std::endl;
                      }
                    else if (seen.find(s_prime) == seen.end())
                      {
                        trace << "  it is not seen, go down" << std::endl;
                        seen.insert(s_prime);
                        tgba_succ_iterator* i = a_->succ_iter(s_prime);
                        i->first();
                        st1.push_front(stack_item(s_prime, i, label, acc));
                      }
                    else if ((acc & covered_acc) != acc)
                      {
                        trace << "  a propagation is needed, start a search"
                              << std::endl;
                        tgba_run::step s = {s_prime, label, acc};
                        if (search(s, target, dead, cycle, covered_acc))
                          return true;
                      }
                    else
                      {
                        trace << "  already seen, pop it" << std::endl;
                      }
                  }
                else
                  delete s_prime;
              }
            else
              {
                trace << "  all the successors have been visited"
                      << std::endl;
                stack_item f_dest(f);
                delete st1.front().it;
                st1.pop_front();
                if (!st1.empty() && (f_dest.acc & covered_acc) != f_dest.acc)
                  {
                    trace << "  a propagation is needed, start a search"
                          << std::endl;
                        tgba_run::step s = {f_dest.s,
                                            f_dest.label,
                                            f_dest.acc};
                        if (search(s, target, dead, cycle, covered_acc))
                          return true;
                  }
                else
                  {
                    trace << "  no propagation needed, pop it" << std::endl;
                  }
              }
          }
        return false;
      }

    bool search(const tgba_run::step& start, const state* target,
                  state_set& dead, tgba_run::steps& cycle, bdd& covered_acc)
      {
        tgba_run::steps path;
        if (start.s->compare(target)==0)
          {
            trace << "  complete the cycle" << std::endl;
            complete_cycle(cycle, covered_acc, start, path, dead);
            return covered_acc == a_->all_acceptance_conditions();
          }

          shortest_path s(a_, target, dead, h_);
          const state* res = s.search(start.s, path);
          if (res)
            {
              assert(res->compare(target) == 0);
              trace << "  complete the cycle" << std::endl;
              complete_cycle(cycle, covered_acc, start, path, dead);
              return covered_acc == a_->all_acceptance_conditions();
            }
          state_set::const_iterator it;
          for (it = s.get_seen().begin(); it != s.get_seen().end(); ++it)
            dead.insert(*it);
          return false;
      }
  };

}

#endif // SPOT_TGBAALGOS_NDFS_RESULT_HH
