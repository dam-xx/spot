#include <sstream>
#include <map>
#include <cassert>
#include "gspnlib.h"
#include "gspn.hh"
#include "ltlvisit/destroy.hh"

namespace spot
{

  gspn_interface::gspn_interface(int argc, char **argv)
  {
    int res = initialize(argc, argv);
    if (res)
      throw gspn_exeption("initialize()", res);
  }

  gspn_interface::~gspn_interface()
  {
    int res = finalize();
    if (res)
      throw gspn_exeption("finalize()", res);
  }


  // state_gspn
  //////////////////////////////////////////////////////////////////////

  class state_gspn: public state
  {
  public:
    state_gspn(State s)
      : state_(s)
    {
    }

    virtual
    ~state_gspn()
    {
    }

    virtual int
    compare(const state* other) const
    {
      const state_gspn* o = dynamic_cast<const state_gspn*>(other);
      assert(o);
      return reinterpret_cast<char*>(o->get_state())
	- reinterpret_cast<char*>(get_state());
    }

    virtual state_gspn* clone() const
    {
      return new state_gspn(get_state());
    }

    State
    get_state() const
    {
      return state_;
    }

  private:
    State state_;
  }; // state_gspn


  // tgba_gspn_private_
  //////////////////////////////////////////////////////////////////////

  struct tgba_gspn_private_
  {
    int refs;			// reference count

    bdd_dict* dict;
    typedef std::pair<AtomicPropKind, bdd> ab_pair;
    typedef std::map<AtomicProp, ab_pair> prop_map;
    prop_map prop_dict;
    AtomicProp *all_indexes;
    size_t index_count;
    const state_gspn* last_state_conds_input;
    bdd last_state_conds_output;


    tgba_gspn_private_(bdd_dict* dict, const gspn_environment& env)
      : refs(1), dict(dict), all_indexes(0), last_state_conds_input(0)
    {
      const gspn_environment::prop_map& p = env.get_prop_map();

      try
	{
	  for (gspn_environment::prop_map::const_iterator i = p.begin();
	       i != p.end(); ++i)
	    {
	      int var = dict->register_proposition(i->second, this);
	      AtomicProp index;
	      int err = prop_index(i->first.c_str(), &index);
	      if (err)
		throw gspn_exeption("prop_index()", err);
	      AtomicPropKind kind;
	      err = prop_kind(index, &kind);
	      if (err)
		throw gspn_exeption("prop_kind()", err);

	      prop_dict[index] = ab_pair(kind, bdd_ithvar(var));
	    }

	  index_count = prop_dict.size();
	  all_indexes = new AtomicProp[index_count];

	  unsigned idx = 0;
	  for (prop_map::const_iterator i = prop_dict.begin();
	       i != prop_dict.end(); ++i)
	    all_indexes[idx++] = i->first;
	}
      catch (...)
	{
	  // If an exception occurs during the loop, we need to clean
	  // all BDD variables which have been registered so far.
	  dict->unregister_all_my_variables(this);
	}
    }

    tgba_gspn_private_::~tgba_gspn_private_()
    {
      dict->unregister_all_my_variables(this);
      if (last_state_conds_input)
	delete last_state_conds_input;
      if (all_indexes)
	delete[] all_indexes;
    }

    bdd index_to_bdd(AtomicProp index) const
    {
      if (index == EVENT_TRUE)
	return bddtrue;
      prop_map::const_iterator i = prop_dict.find(index);
      assert(i != prop_dict.end());
      return i->second.second;
    }

    bdd state_conds(const state_gspn* s)
    {
      // Use cached value if possible.
      if (!last_state_conds_input ||
	  last_state_conds_input->compare(s) != 0)
	{
	  // Build the BDD of the conditions available on this state.
	  unsigned char* cube = 0;
	  // FIXME: This is temporary. We ought to ask only what we need.
	  AtomicProp* want = all_indexes;
	  size_t count = index_count;
	  int res = satisfy(s->get_state(), want, &cube, count);
	  if (res)
	    throw gspn_exeption("satisfy()", res);
	  assert(cube);
	  last_state_conds_output = bddtrue;
	  for (size_t i = 0; i < count; ++i)
	    {
	      bdd v = index_to_bdd(want[i]);
	      last_state_conds_output &= cube[i] ? v : !v;
	    }
	  satisfy_free(cube);

	  if (last_state_conds_input)
	    delete last_state_conds_input;
	  last_state_conds_input = s->clone();
	}
      return last_state_conds_output;
    }
  };


  // tgba_succ_iterator_gspn
  //////////////////////////////////////////////////////////////////////

  class tgba_succ_iterator_gspn: public tgba_succ_iterator
  {
  public:
    tgba_succ_iterator_gspn(bdd state_conds, State state,
			    tgba_gspn_private_* data)
      : state_conds_(state_conds),
	successors_(0),
	size_(0),
	current_(0),
	data_(data)
    {
      int res = succ(state, &successors_, &size_);
      if (res)
	throw gspn_exeption("succ()", res);
      assert(successors_);
      // GSPN is expected to return a looping "dead" transition where
      // there is no successor.
      assert(size_> 0);
    }

    virtual
    ~tgba_succ_iterator_gspn()
    {
      succ_free(successors_);
    }

    virtual void
    first()
    {
      current_ = 0;
    }

    virtual void
    next()
    {
      assert(!done());
      ++current_;
    }

    virtual bool
    done() const
    {
      return current_ >= size_;
    }

    virtual state*
    current_state() const
    {
      return new state_gspn(successors_[current_].s);
    }

    virtual bdd
    current_condition() const
    {
      bdd p = data_->index_to_bdd(successors_[current_].p);
      return state_conds_ & p;
    }

    virtual bdd
    current_accepting_conditions() const
    {
      // There is no accepting conditions in GSPN systems.
      return bddfalse;
    }
  private:
    bdd state_conds_;		/// All conditions known on STATE.
    EventPropSucc* successors_; /// array of successors
    size_t size_;		/// size of successors_
    size_t current_;		/// current position in successors_
    tgba_gspn_private_* data_;
  }; // tgba_succ_iterator_gspn


  // gspn_environment
  //////////////////////////////////////////////////////////////////////

  gspn_environment::gspn_environment()
  {
  }

  gspn_environment::~gspn_environment()
  {
    for (prop_map::iterator i = props_.begin(); i != props_.end(); ++i)
      ltl::destroy(i->second);
  }

  bool
  gspn_environment::declare(const std::string& prop_str)
  {
    if (props_.find(prop_str) != props_.end())
      return false;
    props_[prop_str] = ltl::atomic_prop::instance(prop_str, *this);
    return true;
  }

  ltl::formula*
  gspn_environment::require(const std::string& prop_str)
  {
    prop_map::iterator i = props_.find(prop_str);
    if (i == props_.end())
      return 0;
    // It's an atomic_prop, so we do not have to use the clone() visitor.
    return i->second->ref();
  }

  /// Get the name of the environment.
  const std::string&
  gspn_environment::name()
  {
    static std::string name("gspn environment");
    return name;
  }

  const gspn_environment::prop_map&
  gspn_environment::get_prop_map() const
  {
    return props_;
  }


  // tgba_gspn
  //////////////////////////////////////////////////////////////////////


  tgba_gspn::tgba_gspn(bdd_dict* dict, const gspn_environment& env)
  {
    data_ = new tgba_gspn_private_(dict, env);
  }

  tgba_gspn::tgba_gspn(const tgba_gspn& other)
    : tgba()
  {
    data_ = other.data_;
    ++data_->refs;
  }

  tgba_gspn::~tgba_gspn()
  {
    if (--data_->refs == 0)
      delete data_;
  }

  tgba_gspn&
  tgba_gspn::operator=(const tgba_gspn& other)
  {
    if (&other == this)
      return *this;
    this->~tgba_gspn();
    new (this) tgba_gspn(other);
    return *this;
  }

  state* tgba_gspn::get_init_state() const
  {
    State s;
    int err = initial_state(&s);
    if (err)
      throw gspn_exeption("initial_state()", err);
    return new state_gspn(s);
  }

  tgba_succ_iterator*
  tgba_gspn::succ_iter(const state* state,
		       const state* global_state,
		       const tgba* global_automaton) const
  {
    const state_gspn* s = dynamic_cast<const state_gspn*>(state);
    assert(s);
    (void) global_state;
    (void) global_automaton;
    // FIXME: Should pass global_automaton->support_variables(state)
    // to state_conds.
    bdd state_conds = data_->state_conds(s);
    return new tgba_succ_iterator_gspn(state_conds, s->get_state(), data_);
  }

  bdd
  tgba_gspn::compute_support_conditions(const spot::state* state) const
  {
    const state_gspn* s = dynamic_cast<const state_gspn*>(state);
    assert(s);
    return data_->state_conds(s);
  }

  bdd
  tgba_gspn::compute_support_variables(const spot::state* state) const
  {
    // FIXME: At the time of writing, only tgba_gspn calls
    // support_variables on the root of a product to gather the
    // variables used by all other automata and let GPSN compute only
    // these.  Because support_variables() is recursive over the
    // product treee, tgba_gspn::support_variables should not output
    // all the variables known by GSPN; this would ruin the sole
    // purpose of this function.
    // However this works because we assume there is at most one
    // tgba_gspn automata in a product (a legitimate assumption
    // since the GSPN API is not re-entrant) and only this automata
    // need to call support_variables (now _this_ is shady).
    (void) state;
    return bddtrue;
  }

  bdd_dict*
  tgba_gspn::get_dict() const
  {
    return data_->dict;
  }

  std::string
  tgba_gspn::format_state(const state* state) const
  {
    const state_gspn* s = dynamic_cast<const state_gspn*>(state);
    assert(s);
    // FIXME: We ought to ask GSPN to format the state.
    std::ostringstream os;
    os << s->get_state();
    return os.str();
  }

  bdd
  tgba_gspn::all_accepting_conditions() const
  {
    // There is no accepting conditions in GSPN systems.
    return bddfalse;
  }

  bdd
  tgba_gspn::neg_accepting_conditions() const
  {
    // There is no accepting conditions in GSPN systems.
    return bddtrue;
  }



}
