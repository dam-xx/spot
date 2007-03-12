// Copyright (C) 2004, 2005, 2007  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#include <iostream>
#ifdef TRACE
#define trace std::cerr
#else
#define trace while (0) std::cerr
#endif

#include <list>
#include "misc/hash.hh"
#include "tgba/tgba.hh"
#include "emptiness.hh"
#include "emptiness_stats.hh"
#include "magic.hh"
#include "ndfs_result.hxx"
#include "tgbaalgos/statepipe.hh"
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <errno.h>
#include <sys/wait.h>

namespace spot
{
  namespace
  {
    enum color {WHITE, BLUE, RED, ORANGE};

    /// \brief Emptiness checker on spot::tgba automata having at most one
    /// acceptance condition (i.e. a TBA).
    template <typename heap>
    class magic_search_ : public emptiness_check, public ec_statistics
    {
    public:
      /// \brief Initialize the Magic Search algorithm on the automaton \a a
      ///
      /// \pre The automaton \a a must have at most one acceptance
      /// condition (i.e. it is a TBA).
      magic_search_(const tgba *a, size_t size, option_map o = option_map())
        : emptiness_check(a, o),
          h(size),
          all_cond(a->all_acceptance_conditions())
      {
        assert(a->number_of_acceptance_conditions() <= 1);
      }

      virtual ~magic_search_()
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
            const state* s0 = a_->get_init_state();
            inc_states();
            h.add_new_state(s0, BLUE);
            push(st_blue, s0, bddfalse, bddfalse);
            if (dfs_blue())
              return new magic_search_result(*this, options());
          }
        else
          {
            h.pop_notify(st_red.front().s);
            pop(st_red);
            if (!st_red.empty() && dfs_red())
              return new magic_search_result(*this, options());
            else
              if (dfs_blue())
                return new magic_search_result(*this, options());
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

      virtual bool safe() const
      {
	return heap::Safe;
      }

      const heap& get_heap() const
      {
	return h;
      }

      const stack_type& get_st_blue() const
      {
	return st_blue;
      }

      const stack_type& get_st_red() const
      {
	return st_red;
      }
    private:

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

      /// \brief Stack of the blue dfs.
      stack_type st_blue;

      /// \brief Stack of the red dfs.
      stack_type st_red;

      /// \brief Map where each visited state is colored
      /// by the last dfs visiting it.
      heap h;

      /// State targeted by the red dfs.
      const state* target;

      /// The unique acceptance condition of the automaton \a a.
      bdd all_cond;

      bool dfs_blue()
      {
        while (!st_blue.empty())
          {
            stack_item& f = st_blue.front();
            trace << "DFS_BLUE treats: " << a_->format_state(f.s) << std::endl;
            if (!f.it->done())
              {
                const state *s_prime = f.it->current_state();
                trace << "  Visit the successor: "
                      << a_->format_state(s_prime) << std::endl;
                bdd label = f.it->current_condition();
                bdd acc = f.it->current_acceptance_conditions();
                // Go down the edge (f.s, <label, acc>, s_prime)
                f.it->next();
                inc_transitions();
                typename heap::color_ref c = h.get_color_ref(s_prime);
                if (c.is_white())
                  {
                    trace << "  It is white, go down" << std::endl;
                    inc_states();
                    h.add_new_state(s_prime, BLUE);
                    push(st_blue, s_prime, label, acc);
                  }
                else
                  {
                    if (acc == all_cond && c.get_color() != RED)
                      {
                        // the test 'c.get_color() != RED' is added to limit
                        // the number of runs reported by successive
                        // calls to the check method. Without this
                        // functionnality, the test can be ommited.
                        trace << "  It is blue and the arc is "
                              << "accepting, start a red dfs" << std::endl;
                        target = f.s;
                        c.set_color(RED);
                        push(st_red, s_prime, label, acc);
                        if (dfs_red())
                          return true;
                      }
                    else
                      {
                        trace << "  It is blue or red, pop it" << std::endl;
                        h.pop_notify(s_prime);
                      }
                  }
              }
            else
            // Backtrack the edge
            //        (predecessor of f.s in st_blue, <f.label, f.acc>, f.s)
              {
                trace << "  All the successors have been visited" << std::endl;
                stack_item f_dest(f);
                pop(st_blue);
                typename heap::color_ref c = h.get_color_ref(f_dest.s);
                assert(!c.is_white());
                if (!st_blue.empty() &&
                           f_dest.acc == all_cond && c.get_color() != RED)
                  {
                    // the test 'c.get_color() != RED' is added to limit
                    // the number of runs reported by successive
                    // calls to the check method. Without this
                    // functionnality, the test can be ommited.
                    trace << "  It is blue and the arc from "
                          << a_->format_state(st_blue.front().s)
                          << " to it is accepting, start a red dfs"
                          << std::endl;
                    target = st_blue.front().s;
                    c.set_color(RED);
                    push(st_red, f_dest.s, f_dest.label, f_dest.acc);
                    if (dfs_red())
                      return true;
                  }
                else
                  {
                    trace << "  Pop it" << std::endl;
                    h.pop_notify(f_dest.s);
                  }
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
            trace << "DFS_RED treats: " << a_->format_state(f.s) << std::endl;
            if (!f.it->done())
              {
                const state *s_prime = f.it->current_state();
                trace << "  Visit the successor: "
                      << a_->format_state(s_prime) << std::endl;
                bdd label = f.it->current_condition();
                bdd acc = f.it->current_acceptance_conditions();
                // Go down the edge (f.s, <label, acc>, s_prime)
                f.it->next();
                inc_transitions();
                typename heap::color_ref c = h.get_color_ref(s_prime);
                if (c.is_white())
                  {
                    // If the red dfs find a white here, it must have crossed
                    // the blue stack and the target must be reached soon.
                    // Notice that this property holds only for explicit search.
                    // Collisions in bit-state hashing search can also lead
                    // to the visit of white state. Anyway, it is not necessary
                    // to visit white states either if a cycle can be missed
                    // with bit-state hashing search.
                    trace << "  It is white, pop it" << std::endl;
                    delete s_prime;
                  }
                else if (c.get_color() == BLUE)
                  {
                    trace << "  It is blue, go down" << std::endl;
                    c.set_color(RED);
                    push(st_red, s_prime, label, acc);
                    if (target->compare(s_prime) == 0)
                      return true;
                  }
                else
                  {
                    trace << "  It is red, pop it" << std::endl;
                    h.pop_notify(s_prime);
                  }
              }
            else // Backtrack
              {
                trace << "  All the successors have been visited, pop it"
                      << std::endl;
                h.pop_notify(f.s);
                pop(st_red);
              }
          }
        return false;
      }

      class result_from_stack: public emptiness_check_result,
        public acss_statistics
      {
      public:
        result_from_stack(magic_search_& ms)
          : emptiness_check_result(ms.automaton()), ms_(ms)
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

        unsigned acss_states() const
        {
          return 0;
        }
      private:
        magic_search_& ms_;
      };

#     define FROM_STACK "ar:from_stack"

      class magic_search_result: public emptiness_check_result
      {
      public:
        magic_search_result(magic_search_& m, option_map o = option_map())
          : emptiness_check_result(m.automaton(), o), ms(m)
        {
          if (options()[FROM_STACK])
            computer = new result_from_stack(ms);
          else
            computer = new ndfs_result<magic_search_<heap>, heap>(ms);
        }

        virtual void options_updated(const option_map& old)
        {
          if (old[FROM_STACK] && !options()[FROM_STACK])
            {
              delete computer;
              computer = new ndfs_result<magic_search_<heap>, heap>(ms);
            }
          else if (!old[FROM_STACK] && options()[FROM_STACK])
            {
              delete computer;
              computer = new result_from_stack(ms);
            }
        }

        virtual ~magic_search_result()
        {
          delete computer;
        }

        virtual tgba_run* accepting_run()
        {
          return computer->accepting_run();
        }

        virtual const unsigned_statistics* statistics() const
        {
          return computer->statistics();
        }

      private:
        emptiness_check_result* computer;
        magic_search_& ms;
      };

    }; // magic_search_

    struct checkpool {
      state_pipe* pipes;
      int size;
    };




    /// \brief Emptiness checker on spot::tgba automata having at most one
    /// acceptance condition (i.e. a TBA).
    template <typename heap>
    class magic_search_parallel_ : public emptiness_check, public ec_statistics
    {
      struct successor {
	bdd acc;
	bdd label;
	const spot::state* s;
	successor(bdd acc, bdd label, const spot::state* s):
	  acc(acc), label(label), s(s) {}
      };

      typedef std::list<successor> succ_queue;

      struct stack_item_random
      {

	stack_item_random(const state* s, magic_search_parallel_* msp,
			  bdd l, bdd a)
	  : s(s), label(l), acc(a)
	{
	  tgba_succ_iterator* iter = msp->automaton()->succ_iter(s);
	  for (iter->first(); !iter->done(); iter->next(),
		 msp->inc_transitions())
	    {
	      q.push_back(successor(iter->current_acceptance_conditions(),
				    iter->current_condition(),
				    iter->current_state()));
	      msp->inc_depth();
	    }
	  delete iter;

	}

 	void clean()
 	{
 	  typename succ_queue::iterator i;
 	  for (i = q.begin(); i != q.end(); ++i)
 	    delete i->s;
 	}

	/// The visited state.
	const state* s;
	/// Unprocessed successors of S
	succ_queue q;

	/// The label of the transition traversed to reach \a s
	/// (false for the first one).
	bdd label;
	/// The acceptance set of the transition traversed to reach \a s
	/// (false for the first one).
	bdd acc;
      };

      typedef std::list<stack_item_random> stack_type_random;
    public:
      /// \brief Initialize the Magic Search algorithm on the automaton \a a
      ///
      /// \pre The automaton \a a must have at most one acceptance
      /// condition (i.e. it is a TBA).
      magic_search_parallel_(int me, checkpool& pool,
			     const tgba *a, size_t size,
			     option_map o = option_map())
        : emptiness_check(a, o),
          h(size),
          all_cond(a->all_acceptance_conditions()),
	  me_(me),
	  pool_(pool),
	  orange_hits_(0),
	  items_nw_(0), items_red_(0), items_sent_(0), items_received_(0)

      {
        assert(a->number_of_acceptance_conditions() <= 1);
	stats["orange hits"] =
	  static_cast<spot::unsigned_statistics::unsigned_fun>
	  (&magic_search_parallel_::get_orange_hits);
	stats["items received"] =
	  static_cast<spot::unsigned_statistics::unsigned_fun>
	  (&magic_search_parallel_::get_items_received);
	stats["items sent"] =
	  static_cast<spot::unsigned_statistics::unsigned_fun>
	  (&magic_search_parallel_::get_items_sent);
	stats["items already red"] =
	  static_cast<spot::unsigned_statistics::unsigned_fun>
	  (&magic_search_parallel_::get_items_red);
	stats["items non white"] =
	  static_cast<spot::unsigned_statistics::unsigned_fun>
	  (&magic_search_parallel_::get_items_nw);
      }

      unsigned
      get_orange_hits() const
      {
	return orange_hits_;
      }

      unsigned
      get_items_received() const
      {
	return items_received_;
      }

      unsigned
      get_items_sent() const
      {
	return items_sent_;
      }

      unsigned
      get_items_red() const
      {
	return items_red_;
      }

      unsigned
      get_items_nw() const
      {
	return items_nw_;
      }


      virtual ~magic_search_parallel_()
      {
        // Release all iterators on the stacks.
        while (!st_blue.empty())
          {
            h.pop_notify(st_blue.front().s);
	    st_blue.front().clean();
            st_blue.pop_front();
          }
        while (!st_red.empty())
          {
            h.pop_notify(st_red.front().s);
	    st_red.front().clean();
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
            const state* s0 = a_->get_init_state();
            inc_states();
            h.add_new_state(s0, BLUE);
            push(st_blue, s0, bddfalse, bddfalse);
            if (dfs_blue())
              return new emptiness_check_result(automaton(), options());
          }
        else
          {
            h.pop_notify(st_red.front().s);
            pop(st_red);
            if (!st_red.empty() && dfs_red())
              return new emptiness_check_result(automaton(), options());
            else
              if (dfs_blue())
                return new emptiness_check_result(automaton(), options());
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

      virtual bool safe() const
      {
	return heap::Safe;
      }

      const heap& get_heap() const
      {
	return h;
      }

      const stack_type_random& get_st_blue() const
      {
	return st_blue;
      }

      const stack_type_random& get_st_red() const
      {
	return st_red;
      }
    private:

      void push(stack_type_random& st, const state* s,
		const bdd& label, const bdd& acc)
      {
        inc_depth();
        st.push_front(stack_item_random(s, this, label, acc));
      }

      void pop(stack_type_random& st)
      {
        dec_depth();
        st.pop_front();
      }

      /// \brief Stack of the blue dfs.
      stack_type_random st_blue;

      /// \brief Stack of the red dfs.
      stack_type_random st_red;

      /// \brief Map where each visited state is colored
      /// by the last dfs visiting it.
      heap h;

      /// State targeted by the red dfs.
      const state* target;

      /// The unique acceptance condition of the automaton \a a.
      bdd all_cond;

      bool dfs_blue()
      {
        while (!st_blue.empty())
          {
	    if (pool_.size > 1)
	      process_buffer();

            stack_item_random& f = st_blue.front();
            trace << "DFS_BLUE treats: " << a_->format_state(f.s) << std::endl;
            if (!f.q.empty())
              {
		typename succ_queue::iterator next = f.q.begin();

		int n = rand() % f.q.size();
		while (n--)
		  {
		    ++next;
		    assert(next != f.q.end());
		  }
		successor succ = *next;
		f.q.erase(next);
		dec_depth();

                const state *s_prime = succ.s;
                trace << "  Visit the successor: "
                      << a_->format_state(s_prime) << std::endl;
                bdd label = succ.label;
                bdd acc = succ.acc;
                // Go down the edge (f.s, <label, acc>, s_prime)
                typename heap::color_ref c = h.get_color_ref(s_prime);
                if (c.is_white())
                  {
                    trace << "  It is white, go down" << std::endl;
                    inc_states();
                    h.add_new_state(s_prime, BLUE);
                    push(st_blue, s_prime, label, acc);
                  }
                else
                  {
                    if (acc == all_cond &&
			c.get_color() != RED &&
			c.get_color() != ORANGE )
                      {
                        // the test 'c.get_color() != RED' is added to limit
                        // the number of runs reported by successive
                        // calls to the check method. Without this
                        // functionnality, the test can be ommited.
                        trace << "  It is blue and the arc is "
                              << "accepting, start a red dfs" << std::endl;
                        target = f.s;
                        c.set_color(RED);
                        push(st_red, s_prime, label, acc);
                        if (dfs_red())
                          return true;
			else
			  broadcast(s_prime);
                      }
                    else
                      {
			if (c.get_color() == ORANGE)
			  ++orange_hits_;
                        trace << "  It is blue or red, pop it" << std::endl;
                        h.pop_notify(s_prime);
                      }
                  }
              }
            else
            // Backtrack the edge
            //        (predecessor of f.s in st_blue, <f.label, f.acc>, f.s)
              {
                trace << "  All the successors have been visited" << std::endl;
                stack_item_random f_dest(f);
                pop(st_blue);
                typename heap::color_ref c = h.get_color_ref(f_dest.s);
                assert(!c.is_white());
                if (!st_blue.empty() &&
		    f_dest.acc == all_cond &&
		    c.get_color() != RED &&
		    c.get_color() != ORANGE)
                  {
                    // the test 'c.get_color() != RED' is added to limit
                    // the number of runs reported by successive
                    // calls to the check method. Without this
                    // functionnality, the test can be ommited.
                    trace << "  It is blue and the arc from "
                          << a_->format_state(st_blue.front().s)
                          << " to it is accepting, start a red dfs"
                          << std::endl;
                    target = st_blue.front().s;
                    c.set_color(RED);
                    push(st_red, f_dest.s, f_dest.label, f_dest.acc);
                    if (dfs_red())
                      return true;
		    else
		      broadcast(f_dest.s);
                  }
                else
                  {
                    trace << "  Pop it" << std::endl;
                    h.pop_notify(f_dest.s);
                  }
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
	    if (pool_.size > 1)
	      process_buffer();

            stack_item_random& f = st_red.front();
            trace << "DFS_RED treats: " << a_->format_state(f.s) << std::endl;
            if (!f.q.empty())
              {
		typename succ_queue::iterator next = f.q.begin();

		int n = rand() % f.q.size();
		while (n--)
		  {
		    ++next;
		    assert(next != f.q.end());
		  }
		successor succ = *next;
		f.q.erase(next);
		dec_depth();

                const state *s_prime = succ.s;
                trace << "  Visit the successor: "
                      << a_->format_state(s_prime) << std::endl;
                bdd label = succ.label;
                bdd acc = succ.acc;

		// Go down the edge (f.s, <label, acc>, s_prime)
                typename heap::color_ref c = h.get_color_ref(s_prime);
                if (c.is_white())
                  {
                    // If the red dfs find a white here, it must have crossed
                    // the blue stack and the target must be reached soon.
                    // Notice that this property holds only for explicit search.
                    // Collisions in bit-state hashing search can also lead
                    // to the visit of white state. Anyway, it is not necessary
                    // to visit white states either if a cycle can be missed
                    // with bit-state hashing search.
                    trace << "  It is white, pop it" << std::endl;
                    delete s_prime;
                  }
                else if (c.get_color() == BLUE)
                  {
                    trace << "  It is blue, go down" << std::endl;
                    c.set_color(RED);
                    push(st_red, s_prime, label, acc);
                    if (target->compare(s_prime) == 0)
                      return true;
                  }
                else
                  {
		    if (c.get_color() == ORANGE)
		      ++orange_hits_;
                    trace << "  It is red, pop it" << std::endl;
                    h.pop_notify(s_prime);
                  }
              }
            else // Backtrack
              {
                trace << "  All the successors have been visited, pop it"
                      << std::endl;
                h.pop_notify(f.s);
                pop(st_red);
              }
          }
        return false;
      }

      class result_from_stack: public emptiness_check_result,
        public acss_statistics
      {
      public:
        result_from_stack(magic_search_parallel_& ms)
          : emptiness_check_result(ms.automaton()), ms_(ms)
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

        unsigned acss_states() const
        {
          return 0;
        }
      private:
        magic_search_parallel_& ms_;
      };

#     define FROM_STACK "ar:from_stack"

      class magic_search_result: public emptiness_check_result
      {
      public:
        magic_search_result(magic_search_parallel_& m,
			    option_map o = option_map())
          : emptiness_check_result(m.automaton(), o), ms(m)
        {
          if (options()[FROM_STACK])
            computer = new result_from_stack(ms);
          else
            computer = new ndfs_result<magic_search_parallel_<heap>, heap>(ms);
        }

        virtual void options_updated(const option_map& old)
        {
          if (old[FROM_STACK] && !options()[FROM_STACK])
            {
              delete computer;
              computer = new ndfs_result<magic_search_parallel_<heap>,
		                         heap>(ms);
            }
          else if (!old[FROM_STACK] && options()[FROM_STACK])
            {
              delete computer;
              computer = new result_from_stack(ms);
            }
        }

        virtual ~magic_search_result()
        {
          delete computer;
        }

        virtual tgba_run* accepting_run()
        {
          return computer->accepting_run();
        }

        virtual const unsigned_statistics* statistics() const
        {
          return computer->statistics();
        }

      private:
        emptiness_check_result* computer;
        magic_search_parallel_& ms;
      };

      void
      broadcast(const state* data)
      {
	// std::cout << getpid() << ": broadcast" << std::endl;

	for (int i = 0; i < pool_.size; ++i)
	  {
	    if (i == me_)
	      continue;

	    // FIXME: calling serialize once for the whole list would be faster
	    while (pool_.pipes[i].write_state(data))
	      process_buffer();
	  }
	++items_sent_;
      }

      void
      process_buffer()
      {
	int cpt = 0;
	int redcpt = 0;
	int nwcpt = 0;
	const spot::state* s;

	while ((s = pool_.pipes[me_].read_state(automaton())))
	  {
	    typename heap::color_ref c = h.get_color_ref(s);
	    if (c.is_white())
	      h.add_new_state(s, ORANGE);
	    else
	      {
		++nwcpt;
		if (c.get_color() == RED)
		  ++redcpt;
		else
		  c.set_color(ORANGE);
	      }
	    ++cpt;
	  }

	items_red_ += redcpt;
	items_nw_ += nwcpt;
	items_received_ += cpt;
      }

      int me_;
      checkpool& pool_;
      unsigned orange_hits_;
      unsigned items_nw_;
      unsigned items_red_;
      unsigned items_sent_;
      unsigned items_received_;
    }; // magic_search_parallel_

    class explicit_magic_search_heap
    {
    public:
      enum { Safe = 1 };

      class color_ref
      {
      public:
        color_ref(color* c) :p(c)
          {
          }
        color get_color() const
          {
            return *p;
          }
        void set_color(color c)
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

      void pop_notify(const state*) const
        {
        }

      bool has_been_visited(const state* s) const
        {
          hash_type::const_iterator it = h.find(s);
          return (it != h.end());
        }

      enum { Has_Size = 1 };
      int size() const
        {
          return h.size();
        }

    private:

      typedef Sgi::hash_map<const state*, color,
                state_ptr_hash, state_ptr_equal> hash_type;
      hash_type h;
    };

    class bsh_magic_search_heap
    {
    public:
      enum { Safe = 0 };

      class color_ref
      {
      public:
        color_ref(unsigned char *b, unsigned char o): base(b), offset(o*2)
          {
          }
        color get_color() const
          {
            return color(((*base) >> offset) & 3U);
          }
        void set_color(color c)
          {
            *base =  (*base & ~(3U << offset)) | (c << offset);
          }
        bool is_white() const
          {
            return get_color()==WHITE;
          }
      private:
        unsigned char *base;
        unsigned char offset;
      };

      bsh_magic_search_heap(size_t s)
        {
          size_ = s;
          h = new unsigned char[size_];
          memset(h, WHITE, size_);
        }

      ~bsh_magic_search_heap()
        {
          delete[] h;
        }

      color_ref get_color_ref(const state*& s)
        {
          size_t ha = s->hash();
          return color_ref(&(h[ha%size_]), ha%4);
        }

      void add_new_state(const state* s, color c)
        {
          color_ref cr(get_color_ref(s));
          assert(cr.is_white());
          cr.set_color(c);
        }

      void pop_notify(const state* s) const
        {
          delete s;
        }

      bool has_been_visited(const state* s) const
        {
          size_t ha = s->hash();
          return color((h[ha%size_] >> ((ha%4)*2)) & 3U) != WHITE;
        }

      enum { Has_Size = 0 };

    private:
      size_t size_;
      unsigned char* h;
    };


    class magic_search_parallel_proxy: public emptiness_check
    {
    private:
      checkpool pool_;
    public:
      magic_search_parallel_proxy(const tgba* a, option_map o = option_map())
	: emptiness_check(a, o)
      {
	pool_.size = o.get("parallel", 2);
      }

      ~magic_search_parallel_proxy()
      {
      }

      emptiness_check_result* check ()
      {
	pool_.pipes = new state_pipe[pool_.size];

	size_t size = options().get("bsh");

	std::cout << std::flush;
	std::cerr << std::flush;
	std::clog << std::flush;
	// Create pool_.size children.
	int i;
	for (i = 0; i < pool_.size; ++i)
	  {
	    int pid = fork();
	    if (pid == -1)
	      {
		perror("failed to fork");
		abort();
	      }
	    if (pid == 0)
	      break;
	    std::cout << "FATHER: forking as PID " << pid << std::endl;
	  }


	if (i < pool_.size) // This is the ith child: run the emptiness.
	  {
	    std::cout << getpid() << ": " << i << "th child alive"<< std::endl;
	    srand(getpid());
	    // Close unused FDs.
	    for (int j = 0; j < pool_.size; ++j)
	      {
		if (j != i)
		  pool_.pipes[j].close_read_end();
		else
		  pool_.pipes[j].close_write_end();
	      }

	    emptiness_check* ch;


	      if (size)
		ch =
		  new magic_search_parallel_<bsh_magic_search_heap>
		  (i, pool_, automaton(), size, options());
 	      else
		ch = new magic_search_parallel_<explicit_magic_search_heap>
		  (i, pool_, automaton(), 0, options());
	    std::cout << getpid() << ": child " << i
		      << " running" << std::endl;
	    emptiness_check_result* res = ch->check();
	    if (res)
	      std::cout << getpid() << ": not empty" << std::endl;
	    else
	      std::cout << getpid() << ": empty" << std::endl;


	    spot::unsigned_statistics::stats_map::const_iterator i;
	    spot::unsigned_statistics* s =
	      dynamic_cast<spot::unsigned_statistics*>(ch);
	    assert(s != 0);
	    for (i = s->stats.begin(); i != s->stats.end(); ++i)
	      std::cout << i->first << " = " << (s->*i->second)() << std::endl;

	    exit(res != 0);
	  }

	// This is the father process.
	// We don't need these pipes, they were for children.
	delete[] pool_.pipes;

	std::cout << "FATHER: all children running" << std::endl;

	int res = 0;
	for (int n = pool_.size; n > 0; --n)
	  {
	    std::cout << "FATHER: waiting for " << n
		      << " children to finish" << std::endl;
	    int stat;
	    int child = wait(&stat);
	    if (child == -1)
	      {
		perror("failed to wait for children");
		abort();
	      }
	    if (WIFSIGNALED(stat))
	      std::cout << "FATHER: child with PID " << child << " signaled "
			<< WTERMSIG(stat) << std::endl;
	    else
	      {
		std::cout << "FATHER: child with PID " << child
			  << " terminated " << WEXITSTATUS(stat) << std::endl;
		if (n == pool_.size)
		  res = WEXITSTATUS(stat); // 0: empty, 1: not empty
		else
		  // All successful children should agree.
		  assert(res == WEXITSTATUS(stat));
	      }
	  }
	std::cout << "FATHER: everybody's dead" << std::endl;

	if (res)
	  return new emptiness_check_result(automaton(), options());
	else
	  return 0;
      }

    };

  } // anonymous

  emptiness_check* explicit_magic_search(const tgba *a, option_map o)
  {
    return new magic_search_<explicit_magic_search_heap>(a, 0, o);
  }

  emptiness_check* bit_state_hashing_magic_search(const tgba *a, size_t size,
                                                  option_map o)
  {
    return new magic_search_<bsh_magic_search_heap>(a, size, o);
  }

  emptiness_check*
  magic_search(const tgba *a, option_map o)
  {
    if (o.get("parallel"))
      {
	return new magic_search_parallel_proxy(a, o);
      }
    else
      {
	size_t size = o.get("bsh");
	if (size)
	  return bit_state_hashing_magic_search(a, size, o);
	return explicit_magic_search(a, o);
      }
  }

}
