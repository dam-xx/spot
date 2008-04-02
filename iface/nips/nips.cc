// Copyright (C) 2008  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#include <cassert>
#include "misc/hashfunc.hh"
#include "nips.hh"

namespace spot
{
  namespace
  {
    // Callback for errors
    //////////////////////////////////////////////////////////////////////

    // Callback for error which continues on assertions
    nipsvm_status_t
    search_error_callback(nipsvm_errorcode_t err, nipsvm_pid_t pid,
			  nipsvm_pc_t pc, void *)
    {
      char str[256];

      nipsvm_errorstring (str, sizeof str, err, pid, pc);
      std::cerr << "RUNTIME ERROR (" << err << "): " << str << std::endl;

      // Continue on assertions
      if (err == 9)
	return IC_CONTINUE;

      throw nips_exception(std::string(str), static_cast<int>(err));
      return IC_STOP;
    }

    // Callback for error which fails on assertions
    nipsvm_status_t
    search_error_callback_assert(nipsvm_errorcode_t err, nipsvm_pid_t pid,
				 nipsvm_pc_t pc, void *)
    {
      char str[256];

      nipsvm_errorstring (str, sizeof str, err, pid, pc);
      std::cerr << "RUNTIME ERROR (" << err << "): " << str << std::endl;

      throw nips_exception(std::string(str), static_cast<int>(err));
      return IC_STOP;
    }

    // state_nips
    //////////////////////////////////////////////////////////////////////

    class state_nips: public state
    {
    public:
      state_nips(nipsvm_state_t* s)
      {
	state_nips_init(s);
	nips_state_ = s;
      }

      state_nips(nipsvm_state_t* s, nipsvm_state_t* nips_state)
      {
	state_nips_init(s);
	nips_state_ = nips_state;
      }

      void state_nips_init(nipsvm_state_t* s)
      {
	ref_ = new unsigned(1);
	unsigned long size = nipsvm_state_size(s);
	unsigned long size_buf = size;
	char* state_as_char = new char[size];
	state_ = reinterpret_cast<nipsvm_state_t*>(state_as_char);
	nipsvm_state_copy(size, s, &state_as_char, &size_buf);
      }

      state_nips(const state* other)
	: ref_(new unsigned(1))
      {
	const state_nips* o = dynamic_cast<const state_nips*>(other);
	assert(o);
	ref_ = o->ref_;
	++(*ref_);
	state_ = o->state_;
      }

      virtual
      ~state_nips()
      {
	--(*ref_);
	if (*ref_ == 0)
	{
	  delete[] state_;
	  delete ref_;
	}
      }

      /// Lazy computation for the hash.
      void
      hash_comp()
      {
	size_t size = nipsvm_state_size(get_state());
	hash_ = 0;
	size_t* state = reinterpret_cast<size_t*>(get_state());
	size_t full_size = (size - (size % sizeof (size_t))) / sizeof (size_t);

	unsigned i;
	for (i = 0; i < full_size; ++i)
	  hash_ ^= wang32_hash(state[i]);

	// Hash on the remainder.
	unsigned remainder = 0;
	char* state_in_char = reinterpret_cast<char*>(state);
	size_t init_pos = i * sizeof (size_t);
	unsigned j;
	for (j = 0; j < (size % sizeof (size_t)); ++j)
	  remainder = remainder * 0x100 + state_in_char[init_pos + j];
	for (; j < sizeof (size_t); ++j)
	  remainder *= 0x100;
	hash_ ^= remainder;
      }

      virtual int
      compare(const state* other) const
      {
	const state_nips* o = dynamic_cast<const state_nips*>(other);
	assert(o);
	return reinterpret_cast<char*>(o->get_state())
	  - reinterpret_cast<char*>(get_state());
      }

      virtual size_t
      hash() const
      {
	return reinterpret_cast<char*>(get_state()) - static_cast<char*>(0);
      }

      virtual state_nips* clone() const
      {
	return new state_nips(get_state());
      }

      nipsvm_state_t*
      get_state() const
      {
	return state_;
      }

      nipsvm_state_t*
      get_nips_state() const
      {
	return nips_state_;
      }

    private:
      unsigned* ref_;
      nipsvm_state_t* state_;
      nipsvm_state_t* nips_state_;
    }; // state_nips

    // Callback for successors
    //////////////////////////////////////////////////////////////////////

    nipsvm_status_t
    successor_state_callback(size_t, nipsvm_state_t *succ,
			     nipsvm_transition_information_t *,
			     void *context)
    {
      std::list<state_nips*> *succ_list =
	reinterpret_cast<std::list<state_nips*>*>(context);

      succ_list->push_back(new state_nips(succ));

      return IC_CONTINUE;
    }

    // tgba_succ_iterator_nips
    //////////////////////////////////////////////////////////////////////

    class tgba_succ_iterator_nips : public tgba_succ_iterator
    {
    public:
      typedef std::list<state_nips*> s_list;
      tgba_succ_iterator_nips(const state_nips* parent);
      ~tgba_succ_iterator_nips();
      virtual void first();
      virtual void next();
      virtual bool done() const;
      virtual state* current_state() const;
      virtual bdd current_condition() const;
      virtual bdd current_acceptance_conditions() const;
      s_list* succ_list_get() const;
    private:
      const state_nips* parent_;
      bool has_acceptance_condition_;
      bdd acceptance_condition_;
      s_list* succ_list_;
      s_list::iterator iterator_;
    };

    tgba_succ_iterator_nips::tgba_succ_iterator_nips(const state_nips* parent)
      : parent_(parent), has_acceptance_condition_(false),
	acceptance_condition_(bddfalse), succ_list_(new s_list)
    {
    }

    tgba_succ_iterator_nips::~tgba_succ_iterator_nips()
    {
//       s_list::iterator it = succ_list_->begin();
//       for (; it != succ_list_->end(); ++it)
// 	delete *it;
      delete succ_list_;
    }

    void
    tgba_succ_iterator_nips::first()
    {
      iterator_ = succ_list_->begin();
    }

    void
    tgba_succ_iterator_nips::next()
    {
      ++iterator_;
    }

    bool
    tgba_succ_iterator_nips::done() const
    {
      return iterator_ == succ_list_->end();
    }

    state*
    tgba_succ_iterator_nips::current_state() const
    {
      assert(!done());
      return *iterator_;
    }

    bdd
    tgba_succ_iterator_nips::current_condition() const
    {
      return (nipsvm_state_monitor_acc_or_term(parent_->get_state()) ?
	      bddtrue : bddfalse);
    }

    bdd
    tgba_succ_iterator_nips::current_acceptance_conditions() const
    {
      return (nipsvm_state_monitor_acc_or_term(parent_->get_state()) ?
	      bddtrue : bddfalse);
    }

    tgba_succ_iterator_nips::s_list*
    tgba_succ_iterator_nips::succ_list_get() const
    {
      return succ_list_;
    }

    // tgba_nips
    //////////////////////////////////////////////////////////////////////

    class tgba_nips: public tgba
    {
    public:
      tgba_nips(bdd_dict* dict, nipsvm_t* nipsvm);
      tgba_nips(const tgba_nips& other);
      tgba_nips& operator=(const tgba_nips& other);
      virtual ~tgba_nips();
      virtual state* get_init_state() const;
      virtual tgba_succ_iterator*
      succ_iter(const state* local_state,
		const state* global_state,
		const tgba* global_automaton) const;
      virtual bdd_dict* get_dict() const;
      virtual std::string format_state(const state* state) const;
      virtual bdd all_acceptance_conditions() const;
      virtual bdd neg_acceptance_conditions() const;
    protected:
      virtual bdd compute_support_conditions(const spot::state* state) const;
      virtual bdd compute_support_variables(const spot::state* state) const;
    private:
      bdd_dict* dict_;
      nipsvm_t* nipsvm_;
    }; //tgba_nips

    tgba_nips::tgba_nips(bdd_dict* dict, nipsvm_t* nipsvm)
      : dict_(dict), nipsvm_(nipsvm)
    {
    }

    tgba_nips::tgba_nips(const tgba_nips& other)
      : tgba()
    {
      dict_ = other.dict_;
      nipsvm_ = other.nipsvm_;
    }

    tgba_nips&
    tgba_nips::operator=(const tgba_nips& other)
    {
      if (&other != this)
      {
	dict_ = other.dict_;
	nipsvm_ = other.nipsvm_;
      }
      return *this;
    }

    tgba_nips::~tgba_nips()
    {
    }

    state*
    tgba_nips::get_init_state() const
    {
      return new state_nips(nipsvm_initial_state(nipsvm_));
    }

    tgba_succ_iterator*
    tgba_nips::succ_iter(const state* local_state,
			 const state*,
			 const tgba*) const
    {
      const state_nips* state =
	reinterpret_cast<const state_nips*>(local_state);

      tgba_succ_iterator_nips* iter = new tgba_succ_iterator_nips(state);
      nipsvm_scheduler_iter(nipsvm_, state->get_state(),
			    static_cast<void *>(iter->succ_list_get()));
      return iter;
    }

    bdd_dict* tgba_nips::get_dict() const
    {
      return dict_;
    }

    std::string
    tgba_nips::format_state(const state* state) const
    {
      const state_nips* s = dynamic_cast<const state_nips*>(state);
      unsigned size = global_state_to_str(s->get_state(), 0, 0, 0);
      char buf[size];

      global_state_to_str(s->get_state(), 0, buf, size);

      return std::string(buf);
    }

    bdd
    tgba_nips::all_acceptance_conditions() const
    {
      return bddtrue;
    }

    bdd
    tgba_nips::neg_acceptance_conditions() const
    {
      return bddtrue;
    }

    bdd
    tgba_nips::compute_support_conditions(const spot::state*) const
    {
      return bddtrue;
    }

    bdd
    tgba_nips::compute_support_variables(const spot::state*) const
    {
      return bddtrue;
    }


  } // anonymous

  // nips_interface
  //////////////////////////////////////////////////////////////////////

  nips_interface::nips_interface(bdd_dict* dict,
				 const std::string& filename)
    : dict_(dict)
  {
    bytecode_ = bytecode_load_from_file(filename.c_str(), 0);

    if (bytecode_ == 0)
      throw nips_exception("bytecode_load_from_file()");

    int res = nipsvm_init(&nipsvm_, bytecode_, successor_state_callback,
			  search_error_callback);

    if (res != 0)
      throw nips_exception("nipsvm_init()", res);
  }

  nips_interface::~nips_interface()
  {
    nipsvm_finalize(&nipsvm_);
    bytecode_unload(bytecode_);
  }

  tgba* nips_interface::automaton()
  {
    return new tgba_nips(dict_, &nipsvm_);
  }
}
