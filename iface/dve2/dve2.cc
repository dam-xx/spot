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
#include <sys/stat.h>
#include <unistd.h>

#include "misc/hashfunc.hh"
#include "misc/fixpool.hh"
#include "dve2.hh"


namespace spot
{
  namespace
  {

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
      int (*get_successors)(void* m, int *in, TransitionCB, void *arg);

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
      dve2_state(int s, fixed_size_pool* p)
	: size(s), count(1), pool(p)
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
	pool->deallocate(this);
      }

      size_t hash() const
      {
	return hash_value;
      }

      int compare(const state* other) const
      {
	if (this == other)
	  return 0;
	const dve2_state* o = down_cast<const dve2_state*>(other);
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
      }

    public:
      int size;
      mutable unsigned count;
      size_t hash_value;
      fixed_size_pool* pool;
      int vars[0];
    };

    ////////////////////////////////////////////////////////////////////////
    // CALLBACK FUNCTION for transitions.

    struct callback_context
    {
      typedef std::vector<dve2_state*> transitions_t; // FIXME: Vector->List
      transitions_t transitions;
      int state_size;
      fixed_size_pool* pool;

      ~callback_context()
      {
	callback_context::transitions_t::const_iterator it;
	for (it = transitions.begin(); it != transitions.end(); ++it)
	  (*it)->destroy();
      }
    };

    void transition_callback(void* arg, transition_info_t*, int *dst)
    {
      callback_context* ctx = static_cast<callback_context*>(arg);
      dve2_state* out =
	new(ctx->pool->allocate()) dve2_state(ctx->state_size, ctx->pool);
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
    // PREDICATE EVALUATION

    typedef enum { OP_EQ, OP_NE, OP_LT, OP_GT, OP_LE, OP_GE } relop;

    struct one_prop
    {
      int var_num;
      relop op;
      int val;
      int bddvar;  // if "var_num op val" is true, output bddvar,
		   // else its negation
    };
    typedef std::vector<one_prop> prop_set;


    struct var_info
    {
      int num;
      int type;
    };


    int
    convert_aps(const ltl::atomic_prop_set* aps,
		const dve2_interface* d,
		bdd_dict* dict,
		const ltl::formula* dead,
		prop_set& out)
    {
      int errors = 0;

      int state_size = d->get_state_variable_count();
      typedef std::map<std::string, var_info> val_map_t;
      val_map_t val_map;

      for (int i = 0; i < state_size; ++i)
	{
	  const char* name = d->get_state_variable_name(i);
	  int type = d->get_state_variable_type(i);
	  var_info v = { i , type };
	  val_map[name] = v;
	}

      int type_count = d->get_state_variable_type_count();
      typedef std::map<std::string, int> enum_map_t;
      std::vector<enum_map_t> enum_map(type_count);
      for (int i = 0; i < type_count; ++i)
	{
	  int enum_count = d->get_state_variable_type_value_count(i);
	  for (int j = 0; j < enum_count; ++j)
	    enum_map[i]
	      .insert(std::make_pair(d->get_state_variable_type_value(i, j),
				     j));
	}

      for (ltl::atomic_prop_set::const_iterator ap = aps->begin();
	   ap != aps->end(); ++ap)
	{
	  if (*ap == dead)
	    continue;

	  std::string str = (*ap)->name();
	  const char* s = str.c_str();

	  // Skip any leading blank.
	  while (*s && (*s == ' ' || *s == '\t'))
	    ++s;
	  if (!*s)
	    {
	      std::cerr << "Proposition `" << str
			<< "' cannot be parsed." << std::endl;
	      ++errors;
	      continue;
	    }


	  char* name = (char*) malloc(str.size() + 1);
	  char* name_p = name;
	  char* lastdot = 0;
	  while (*s && (*s != '=') && *s != '<' && *s != '!'  && *s != '>')
	    {

	      if (*s == ' ' || *s == '\t')
		++s;
	      else
		{
		  if (*s == '.')
		    lastdot = name_p;
		  *name_p++ = *s++;
		}
	    }
	  *name_p = 0;

	  if (name == name_p)
	    {
	      std::cerr << "Proposition `" << str
			<< "' cannot be parsed." << std::endl;
	      free(name);
	      ++errors;
	      continue;
	    }

	  // Lookup the name
	  val_map_t::const_iterator ni = val_map.find(name);
	  if (ni == val_map.end())
	    {
	      // We may have a name such as X.Y.Z
	      // If it is not a known variable, it might mean
	      // an enumerated variable X.Y with value Z.
	      if (lastdot)
		{
		  *lastdot++ = 0;
		  ni = val_map.find(name);
		}

	      if (ni == val_map.end())
		{
		  std::cerr << "No variable `" << name
			    << "' found in model (for proposition `"
			    << str << "')." << std::endl;
		  free(name);
		  ++errors;
		  continue;
		}

	      // We have found the enumerated variable, and lastdot is
	      // pointing to its expected value.
	      int type_num = ni->second.type;
	      enum_map_t::const_iterator ei = enum_map[type_num].find(lastdot);
	      if (ei == enum_map[type_num].end())
		{
		  std::cerr << "No state `" << lastdot
			    << "' known for variable `"
			    << name << "'." << std::endl;
		  std::cerr << "Possible states are:";
		  for (ei = enum_map[type_num].begin();
		       ei != enum_map[type_num].end(); ++ei)
		    std::cerr << " " << ei->first;
		  std::cerr << std::endl;

		  free(name);
		  ++errors;
		  continue;
		}

	      // At this point, *s should be 0.
	      if (*s)
		{
		  std::cerr << "Trailing garbage `" << s
			    << "' at end of proposition `"
			    << str << "'." << std::endl;
		  free(name);
		  ++errors;
		  continue;
		}

	      // Record that X.Y must be equal to Z.
	      int v = dict->register_proposition(*ap, d);
	      one_prop p = { ni->second.num, OP_EQ, ei->second, v };
	      out.push_back(p);
	      free(name);
	      continue;
	    }

	  int var_num = ni->second.num;

	  if (!*s)		// No operator?  Assume "!= 0".
	    {
	      int v = dict->register_proposition(*ap, d);
	      one_prop p = { var_num, OP_NE, 0, v };
	      out.push_back(p);
	      free(name);
	      continue;
	    }

	  relop op;

	  switch (*s)
	    {
	    case '!':
	      if (s[1] != '=')
		goto report_error;
	      op = OP_NE;
	      s += 2;
	      break;
	    case '=':
	      if (s[1] != '=')
		goto report_error;
	      op = OP_EQ;
	      s += 2;
	      break;
	    case '<':
	      if (s[1] == '=')
		{
		  op = OP_LE;
		  s += 2;
		}
	      else
		{
		  op = OP_LT;
		  ++s;
		}
	      break;
	    case '>':
	      if (s[1] == '=')
		{
		  op = OP_GE;
		  s += 2;
		}
	      else
		{
		  op = OP_GT;
		  ++s;
		}
	      break;
	    default:
	    report_error:
	      std::cerr << "Unexpected `" << s
			<< "' while parsing atomic proposition `" << str
			<< "'." << std::endl;
	      ++errors;
	      free(name);
	      continue;
	    }

	  while (*s && (*s == ' ' || *s == '\t'))
	    ++s;

	  int val;
	  int type_num = ni->second.type;
	  if (type_num == 0 || (*s >= '0' && *s <= '9') || *s == '-')
	    {
	      char* s_end;
	      val = strtol(s, &s_end, 10);
	      if (s == s_end)
		{
		  std::cerr << "Failed to parse `" << s
			    << "' as an integer." << std::endl;
		  ++errors;
		  free(name);
		  continue;
		}
	      s = s_end;
	    }
	  else
	    {
	      // We are in a case such as P_0 == S, trying to convert
	      // the string S into an integer.
	      const char* end = s;
	      while (*end && *end != ' ' && *end != '\t')
		++end;
	      std::string st(s, end);

	      // Lookup the string.
	      enum_map_t::const_iterator ei = enum_map[type_num].find(st);
	      if (ei == enum_map[type_num].end())
		{
		  std::cerr << "No state `" << st
			    << "' known for variable `"
			    << name << "'." << std::endl;
		  std::cerr << "Possible states are:";
		  for (ei = enum_map[type_num].begin();
		       ei != enum_map[type_num].end(); ++ei)
		    std::cerr << " " << ei->first;
		  std::cerr << std::endl;

		  free(name);
		  ++errors;
		  continue;
		}
	      s = end;
	      val = ei->second;
	    }

	  free(name);

	  while (*s && (*s == ' ' || *s == '\t'))
	    ++s;
	  if (*s)
	    {
	      std::cerr << "Unexpected `" << s
			<< "' while parsing atomic proposition `" << str
			<< "'." << std::endl;
		  ++errors;
		  continue;
	    }


	  int v = dict->register_proposition(*ap, d);
	  one_prop p = { var_num, op, val, v };
	  out.push_back(p);
	}

      return errors;
    }

    ////////////////////////////////////////////////////////////////////////
    // KRIPKE

    class dve2_kripke: public kripke
    {
    public:

      dve2_kripke(const dve2_interface* d, bdd_dict* dict, const prop_set* ps,
		  const ltl::formula* dead)
	: d_(d),
	  state_size_(d_->get_state_variable_count()),
	  dict_(dict), ps_(ps),
	  statepool_(sizeof(dve2_state) + state_size_ * sizeof(int)),
	  state_condition_last_state_(0), state_condition_last_cc_(0)
      {
	vname_ = new const char*[state_size_];
	for (int i = 0; i < state_size_; ++i)
	  vname_[i] = d_->get_state_variable_name(i);

	// Register the "dead" proposition.  There are three cases to
	// consider:
	//  * If DEAD is "false", it means we are not interested in finite
	//    sequences of the system.
	//  * If DEAD is "true", we want to check finite sequences as well
	//    as infinite sequences, but do not need to distinguish them.
	//  * If DEAD is any other string, this is the name a property
	//    that should be true when looping on a dead state, and false
	//    otherwise.
	// We handle these three cases by setting ALIVE_PROP and DEAD_PROP
	// appropriately.  ALIVE_PROP is the bdd that should be ANDed
	// to all transitions leaving a live state, while DEAD_PROP should
	// be ANDed to all transitions leaving a dead state.
	if (dead == ltl::constant::false_instance())
	  {
	    alive_prop = bddtrue;
	    dead_prop = bddfalse;
	  }
	else if (dead == ltl::constant::false_instance())
	  {
	    alive_prop = bddtrue;
	    dead_prop = bddtrue;
	  }
	else
	  {
	    int var = dict->register_proposition(dead, d_);
	    dead_prop = bdd_ithvar(var);
	    alive_prop = bdd_nithvar(var);
	  }
      }

      ~dve2_kripke()
      {
	delete[] vname_;
	lt_dlclose(d_->handle);

	dict_->unregister_all_my_variables(d_);

	delete d_;
	delete ps_;
	lt_dlexit();

	if (state_condition_last_state_)
	  state_condition_last_state_->destroy();
	delete state_condition_last_cc_; // Might be 0 already.
      }

      virtual
      state* get_init_state() const
      {
	fixed_size_pool* p = const_cast<fixed_size_pool*>(&statepool_);
	dve2_state* res = new(p->allocate()) dve2_state(state_size_, p);
	d_->get_initial_state(res->vars);
	res->compute_hash();
	return res;
      }

      bdd
      compute_state_condition(const dve2_state* s) const
      {
	// If we just computed it, don't do it twice.
	if (s == state_condition_last_state_)
	  return state_condition_last_cond_;

	if (state_condition_last_state_)
	  {
	    state_condition_last_state_->destroy();
	    delete state_condition_last_cc_; // Might be 0 already.
	    state_condition_last_cc_ = 0;
	  }

	bdd res = bddtrue;
	for (prop_set::const_iterator i = ps_->begin();
	     i != ps_->end(); ++i)
	  {
	    int l = s->vars[i->var_num];
	    int r = i->val;


	    bool cond = false;
	    switch (i->op)
	      {
	      case OP_EQ:
		cond = (l == r);
		break;
	      case OP_NE:
		cond = (l != r);
		break;
	      case OP_LT:
		cond = (l < r);
		break;
	      case OP_GT:
		cond = (l > r);
		break;
	      case OP_LE:
		cond = (l <= r);
		break;
	      case OP_GE:
		cond = (l >= r);
		break;
	      }

	    if (cond)
	      res &= bdd_ithvar(i->bddvar);
	    else
	      res &= bdd_nithvar(i->bddvar);
	  }

	callback_context* cc = new callback_context;
	cc->state_size = state_size_;
	cc->pool = const_cast<fixed_size_pool*>(&statepool_);
	int t = d_->get_successors(0, const_cast<int*>(s->vars),
				   transition_callback, cc);
	assert((unsigned)t == cc->transitions.size());
	state_condition_last_cc_ = cc;

	if (t)
	  {
	    res &= alive_prop;
	  }
	else
	  {
	    res &= dead_prop;

	    // Add a self-loop to dead-states if we care about these.
	    if (res != bddfalse)
	      cc->transitions.push_back(s->clone());
	  }

	state_condition_last_cond_ = res;
	state_condition_last_state_ = s->clone();

	return res;
      }

      virtual
      dve2_succ_iterator*
      succ_iter(const state* local_state,
		const state*, const tgba*) const
      {
	const dve2_state* s = down_cast<const dve2_state*>(local_state);
	assert(s);

	// This may also compute successors in state_condition_last_cc
	bdd scond = compute_state_condition(s);

	callback_context* cc;

	if (state_condition_last_cc_)
	  {
	    cc = state_condition_last_cc_;
	    state_condition_last_cc_ = 0; // Now owned by the iterator.
	  }
	else
	  {
	    cc = new callback_context;
	    cc->state_size = state_size_;
	    cc->pool = const_cast<fixed_size_pool*>(&statepool_);
	    int t = d_->get_successors(0, const_cast<int*>(s->vars),
				       transition_callback, cc);
	    assert((unsigned)t == cc->transitions.size());

	    // Add a self-loop to dead-states if we care about these.
	    if (t == 0 && scond != bddfalse)
	      cc->transitions.push_back(s->clone());
	  }

	return new dve2_succ_iterator(cc, scond);
      }

      virtual
      bdd
      state_condition(const state* st) const
      {
	const dve2_state* s = down_cast<const dve2_state*>(st);
	assert(s);
	return compute_state_condition(s);
      }

      virtual
      std::string format_state(const state *st) const
      {
	const dve2_state* s = down_cast<const dve2_state*>(st);
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
      const prop_set* ps_;
      bdd alive_prop;
      bdd dead_prop;

      fixed_size_pool statepool_;

      // This cache is used to speedup repeated calls to state_condition()
      // and get_succ().
      // If state_condition_last_state_ != 0, then state_condition_last_cond_
      // contain its (recently computed) condition.  If additionally
      // state_condition_last_cc_ != 0, then it contains the successors.
      mutable const dve2_state* state_condition_last_state_;
      mutable bdd state_condition_last_cond_;
      mutable callback_context* state_condition_last_cc_;
    };

  }


  ////////////////////////////////////////////////////////////////////////////
  // LOADER


  // Call divine to compile "foo.dve" as "foo.dve2C" if the latter
  // does not exist already or is older.
  bool
  compile_dve2(std::string& filename, bool verbose)
  {

    std::string command = "divine compile --ltsmin " + filename;

    struct stat s;
    if (stat(filename.c_str(), &s) != 0)
      {
	if (verbose)
	  {
	    std::cerr << "Cannot open " << filename << std::endl;
	    return true;
	  }
      }

    std::string old = filename;
    filename += "2C";

    // Remove any directory, because the new file will
    // be compiled in the current directory.
    size_t pos = filename.find_last_of("/\\");
    if (pos != std::string::npos)
      filename = "./" + filename.substr(pos + 1);

    struct stat d;
    if (stat(filename.c_str(), &d) == 0)
      if (s.st_mtime < d.st_mtime)
	// The dve2C is up-to-date, no need to recompile it.
	return false;

    int res = system(command.c_str());
    if (res)
      {
	if (verbose)
	  std::cerr << "Execution of `" << command.c_str()
		    << "' returned exit code " << WEXITSTATUS(res)
		    << "." << std::endl;
	return true;
      }
    return false;
  }


  kripke*
  load_dve2(const std::string& file_arg, bdd_dict* dict,
	    const ltl::atomic_prop_set* to_observe,
	    const ltl::formula* dead,
	    bool verbose)
  {
    std::string file;
    if (file_arg.find_first_of("/\\") != std::string::npos)
      file = file_arg;
    else
      file = "./" + file_arg;

    std::string ext = file.substr(file.find_last_of("."));
    if (ext == ".dve")
      {
	if (compile_dve2(file, verbose))
	  {
	    if (verbose)
	      std::cerr << "Failed to compile `" << file_arg
			<< "'." << std::endl;
	    return 0;
	  }
      }
    else if (ext != ".dve2C")
      {
	if (verbose)
	  std::cerr << "Unknown extension `" << ext
		    << "'.  Use `.dve' or `.dve2C'." << std::endl;
	return 0;
      }

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
      lt_dlsym(h, "get_state_variable_type_name");
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

    prop_set* ps = new prop_set;
    int errors = convert_aps(to_observe, d, dict, dead, *ps);
    if (errors)
      {
	delete ps;
	dict->unregister_all_my_variables(d);
	delete d;
	lt_dlexit();
	return 0;
      }

    return new dve2_kripke(d, dict, ps, dead);
  }
}
