#include <sstream>
#include <map>
#include <cassert>
#include "gspnlib.h"
#include "gspn.hh"
#include "ltlvisit/destroy.hh"

namespace spot
{

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

    tgba_gspn_private_(bdd_dict* dict, const gspn_environment& env)
      : refs(0), dict(dict)
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
		throw gspn_exeption(err);
	      AtomicPropKind kind;
	      err = prop_kind(index, &kind);
	      if (err)
		throw gspn_exeption(err);

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
    }

    bdd index_to_bdd(AtomicProp index) const
    {
      if (index == EVENT_TRUE)
	return bddtrue;
      prop_map::const_iterator i = prop_dict.find(index);
      assert(i != prop_dict.end());
      return i->second.second;
    }
  };

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
      return o->get_state() - get_state();
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
      AtomicProp want[] = { EVENT_TRUE };
      int res = succ(state, want, sizeof(want) / sizeof(*want),
		     &successors_, &size_);
      if (res)
	throw res;
      assert(successors_);
      // GSPN is expected to return a looping "dead" transition where
      // there is no successor,
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
    size_t size_;    		/// size of successors_
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
      throw gspn_exeption(err);
    return new state_gspn(s);
  }

  tgba_succ_iterator*
  tgba_gspn::succ_iter(const state* state) const
  {
    const state_gspn* s = dynamic_cast<const state_gspn*>(state);
    assert(s);

    // Build the BDD of the conditions available on this state.
    unsigned char* cube = 0;
    int res = satisfy(s->get_state(),
		      data_->all_indexes, &cube, data_->index_count);
    if (res)
      throw res;
    assert(cube);
    bdd state_conds = bddtrue;
    for (size_t i = 0; i < data_->index_count; ++i)
      {
	state_conds &= data_->index_to_bdd(cube[i]);
      }
    satisfy_free(cube);

    return new tgba_succ_iterator_gspn(state_conds, s->get_state(), data_);
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
