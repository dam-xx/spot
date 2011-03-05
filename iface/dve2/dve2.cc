// Copyright (C) 2011 Laboratoire de Recherche et Developpement de
// l'Epita (LRDE)
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

#include <ltdl.h>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <sstream>

#include "misc/hashfunc.hh"
#include "dve2.hh"

namespace spot
{
  namespace {

    ////////////////////////////////////////////////////////////////////////
    // DVE2 --ltsmin interface

    typedef struct transition_info {
      int* labels; // edge labels, NULL, or pointer to the edge label(s)
      int  group;  // holds transition group or -1 if unknown
    } transition_info_t;

    typedef void (*TransitionCB)(void *ctx,
				 transition_info_t *transition_info,
				 int *dst);

    struct dve2_interface
    {
      lt_dlhandle handle;	// handle to the dynamic library
      void (*get_initial_state)(void *to);
      int (*have_property)();
      int (*get_successors)( void* m, int *in, TransitionCB, void *arg );

      int (*get_state_variable_count)();
      const char* (*get_state_variable_name)(int var);
      int (*get_state_variable_type)(int var);
      int (*get_state_variable_type_count)();
      const char* (*get_state_variable_type_name)(int type);
      int (*get_state_variable_type_value_count)(int type);
      const char* (*get_state_variable_type_value)(int type, int value);
      int (*get_transition_count)();
    };

    ////////////////////////////////////////////////////////////////////////
    // STATE

    struct dve2_state: public state
    {
      int* vars;
      int size;
      mutable int count;
      size_t hash_value;

      dve2_state(int s)
	: vars(new int[s]), size(s), count(1)
      {
      }

      void compute_hash()
      {
	hash_value = 0;
	for (int i = 0; i < size; ++i)
	  hash_value = wang32_hash(hash_value ^ vars[i]);
      }

      dve2_state* clone() const
      {
	++count;
	return const_cast<dve2_state*>(this);
      }

      void destroy() const
      {
	if (--count)
	  return;
	delete this;
      }

      size_t hash() const
      {
	return hash_value;
      }

      int compare(const state* other) const
      {
	if (this == other)
	  return 0;
	const dve2_state* o = dynamic_cast<const dve2_state*>(other);
	assert(o);
	if (hash_value < o->hash_value)
	  return -1;
	if (hash_value > o->hash_value)
	  return 1;
	return memcmp(vars, o->vars, size * sizeof(*vars));
      }

    private:

      ~dve2_state()
      {
	delete[] vars;
      }

    };

    ////////////////////////////////////////////////////////////////////////
    // CALLBACK FUNCTION for transitions.

    struct callback_context
    {
      typedef std::vector<dve2_state*> transitions_t;
      transitions_t transitions;
      int state_size;
    };

    void transition_callback(void* arg, transition_info_t*, int *dst)
    {
      callback_context* ctx = static_cast<callback_context*>(arg);
      dve2_state* out = new dve2_state(ctx->state_size);
      memcpy(out->vars, dst, ctx->state_size * sizeof(int));
      out->compute_hash();
      ctx->transitions.push_back(out);
    }

    ////////////////////////////////////////////////////////////////////////
    // SUCC_ITERATOR

    class dve2_succ_iterator: public kripke_succ_iterator
    {
    public:

      dve2_succ_iterator(const callback_context* cc,
			 bdd cond)
	: kripke_succ_iterator(cond), cc_(cc)
      {
      }

      ~dve2_succ_iterator()
      {
	for (it_ = cc_->transitions.begin();
	     it_ != cc_->transitions.end();
	     ++it_)
	  (*it_)->destroy();
	delete cc_;
      }

      virtual
      void first()
      {
	it_ = cc_->transitions.begin();
      }

      virtual
      void next()
      {
	++it_;
      }

      virtual
      bool done() const
      {
	return it_ == cc_->transitions.end();
      }

      virtual
      state* current_state() const
      {
	return (*it_)->clone();
      }

    private:
      const callback_context* cc_;
      callback_context::transitions_t::const_iterator it_;
    };

    ////////////////////////////////////////////////////////////////////////
    // KRIPKE

    class dve2_kripke: public kripke
    {
    public:

      dve2_kripke(const dve2_interface* d, bdd_dict* dict)
	: d_(d), dict_(dict)
      {
	state_size_ = d_->get_state_variable_count();

	vname_ = new const char*[state_size_];
	for (int i = 0; i < state_size_; ++i)
	  vname_[i] = d_->get_state_variable_name(i);
      }

      ~dve2_kripke()
      {
	delete[] vname_;
	delete d_;
      }

      virtual
      state* get_init_state() const
      {
	dve2_state* res = new dve2_state(state_size_);
	d_->get_initial_state(res->vars);
	res->compute_hash();
	return res;
      }

      virtual
      dve2_succ_iterator*
      succ_iter(const state* local_state,
		const state*, const tgba*) const
      {
	const dve2_state* s = dynamic_cast<const dve2_state*>(local_state);
	assert(s);

	callback_context* cc = new callback_context;
	cc->state_size = state_size_;
	int t = d_->get_successors(0, s->vars, transition_callback, cc);
	(void) t;
	assert((unsigned)t == cc->transitions.size());

	return new dve2_succ_iterator(cc, bddtrue);
      }

      virtual
      bdd
      state_condition(const state* s) const
      {
	(void) s;
	return bddfalse;
      }

      virtual
      std::string format_state(const state *st) const
      {
	const dve2_state* s = dynamic_cast<const dve2_state*>(st);
	assert(s);

	std::stringstream res;

	if (state_size_ == 0)
	  return "empty state";

	int i = 0;
	for (;;)
	  {
	    res << vname_[i] << "=" << s->vars[i];
	    ++i;
	    if (i == state_size_)
	      break;
	    res << ", ";
	  }
	return res.str();
      }

      virtual
      spot::bdd_dict* get_dict() const
      {
	return dict_;
      }

    private:
      const dve2_interface* d_;
      int state_size_;
      bdd_dict* dict_;
      const char** vname_;
    };

  }


  ////////////////////////////////////////////////////////////////////////////
  // LOADER

  kripke* load_dve2(const std::string& file_arg, bdd_dict* dict, bool verbose)
  {
    std::string file;
    if (file_arg.find_first_of("/\\") != std::string::npos)
      file = file_arg;
    else
      file = "./" + file_arg;

    if (lt_dlinit())
      {
	if (verbose)
	  std::cerr << "Failed to initialize libltdl." << std::endl;
	return 0;
      }

    lt_dlhandle h = lt_dlopen(file.c_str());
    if (!h)
      {
	if (verbose)
	  std::cerr << "Failed to load `" << file << "'." << std::endl;
	lt_dlexit();
	return 0;
      }

    dve2_interface* d = new dve2_interface;
    d->handle = h;

    d->get_initial_state = (void (*)(void*))
      lt_dlsym(h, "get_initial_state");
    d->have_property = (int (*)())
      lt_dlsym(h, "have_property");
    d->get_successors = (int (*)(void*, int*, TransitionCB, void*))
      lt_dlsym(h, "get_successors");
    d->get_state_variable_count = (int (*)())
      lt_dlsym(h, "get_state_variable_count");
    d->get_state_variable_name = (const char* (*)(int))
      lt_dlsym(h, "get_state_variable_name");
    d->get_state_variable_type = (int (*)(int))
      lt_dlsym(h, "get_state_variable_type");
    d->get_state_variable_type_count = (int (*)())
      lt_dlsym(h, "get_state_variable_type_count");
    d->get_state_variable_type_name = (const char* (*)(int))
      lt_dlsym(h, "get_state_variable_type_name" );
    d->get_state_variable_type_value_count = (int (*)(int))
      lt_dlsym(h, "get_state_variable_type_value_count");
    d->get_state_variable_type_value = (const char* (*)(int, int))
      lt_dlsym(h, "get_state_variable_type_value");
    d->get_transition_count = (int (*)())
      lt_dlsym(h, "get_transition_count");

    if (!(d->get_initial_state
	  && d->have_property
	  && d->get_successors
	  && d->get_state_variable_count
	  && d->get_state_variable_name
	  && d->get_state_variable_type
	  && d->get_state_variable_type_count
	  && d->get_state_variable_type_name
	  && d->get_state_variable_type_value_count
	  && d->get_state_variable_type_value
	  && d->get_transition_count))
      {
	if (verbose)
	  std::cerr << "Failed to resolve some symbol while loading `"
		    << file << "'" << std::endl;
	delete d;
	lt_dlexit();
	return 0;
      }

    if (d->have_property())
      {
	if (verbose)
	  std::cerr << "Model with an embedded property are not supported."
		    << std::endl;
	delete d;
	lt_dlexit();
	return 0;
      }

    return new dve2_kripke(d, dict);
  }
}
