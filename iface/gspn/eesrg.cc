#include <sstream>
#include <cstring>
#include <map>
#include <cassert>
#include "eesrg.hh"
#include <gspnlib.h>
#include "misc/bddlt.hh"
#include <bdd.h>

namespace spot
{

  gspn_eesrg_interface::gspn_eesrg_interface(int argc, char **argv)
  {
    int res = initialize(argc, argv);
    if (res)
      throw gspn_exeption("initialize()", res);
  }

  gspn_eesrg_interface::~gspn_eesrg_interface()
  {
    int res = finalize();
    if (res)
      throw gspn_exeption("finalize()", res);
  }


  // state_gspn_eesrg
  //////////////////////////////////////////////////////////////////////

  class state_gspn_eesrg: public state
  {
  public:
    state_gspn_eesrg(State left, const state* right)
      : left_(left), right_(right)
    {
    }

    virtual
    ~state_gspn_eesrg()
    {
      delete right_;
    }

    virtual int
    compare(const state* other) const
    {
      const state_gspn_eesrg* o = dynamic_cast<const state_gspn_eesrg*>(other);
      assert(o);
      int res = (reinterpret_cast<char*>(o->left())
		 - reinterpret_cast<char*>(left()));
      if (res != 0)
	return res;
      return right_->compare(o->right());
    }

    virtual size_t
    hash() const
    {
      return (reinterpret_cast<char*>(left())
	      - static_cast<char*>(0)) << 10 + right_->hash();
    }

    virtual state_gspn_eesrg* clone() const
    {
      return new state_gspn_eesrg(left(), right());
    }

    State
    left() const
    {
      return left_;
    }

    const state*
    right() const
    {
      return right_;
    }

  private:
    State left_;
    const state* right_;
  }; // state_gspn_eesrg



  // tgba_gspn_eesrg_private_
  //////////////////////////////////////////////////////////////////////

  struct tgba_gspn_eesrg_private_
  {
    int refs;			// reference count

    bdd_dict* dict;
    typedef std::map<int, AtomicProp> prop_map;
    prop_map prop_dict;

    signed char* all_props;

    size_t prop_count;
    const tgba* operand;

    tgba_gspn_eesrg_private_(bdd_dict* dict, const gspn_environment& env,
			     const tgba* operand)
      : refs(1), dict(dict), all_props(0),
	operand(operand)
    {
      const gspn_environment::prop_map& p = env.get_prop_map();

      try
	{
	  AtomicProp max_prop = 0;

	  for (gspn_environment::prop_map::const_iterator i = p.begin();
	       i != p.end(); ++i)
	    {
	      int var = dict->register_proposition(i->second, this);
	      AtomicProp index;
	      int err = prop_index(i->first.c_str(), &index);
	      if (err)
		throw gspn_exeption("prop_index(" + i->first + ")", err);

	      prop_dict[var] = index;

	      max_prop = std::max(max_prop, index);
	    }
	  prop_count = 1 + max_prop;
	  all_props = new signed char[prop_count];
	}
      catch (...)
	{
	  // If an exception occurs during the loop, we need to clean
	  // all BDD variables which have been registered so far.
	  dict->unregister_all_my_variables(this);
	  throw;
	}
    }

    tgba_gspn_eesrg_private_::~tgba_gspn_eesrg_private_()
    {
      dict->unregister_all_my_variables(this);
      if (all_props)
	delete[] all_props;
    }

  };


  // tgba_succ_iterator_gspn_eesrg
  //////////////////////////////////////////////////////////////////////

  class tgba_succ_iterator_gspn_eesrg: public tgba_succ_iterator
  {
  public:
    tgba_succ_iterator_gspn_eesrg(State state,
				  tgba_gspn_eesrg_private_* data,
				  tgba_succ_iterator* operand)
      : state_(state),
	operand_(operand),
	all_conds_(bddfalse),
	successors_(0),
	size_(0),
	current_(0),
	data_(data)
    {
    }

    virtual
    ~tgba_succ_iterator_gspn_eesrg()
    {
      if (operand_)
	delete operand_;
      if (successors_)
	succ_free(successors_);
    }


    void
    step()
    {
      if (current_ < size_)
	{
	  ++current_;
	  return;
	}

      if (successors_)
	succ_free(successors_);
      current_ = 0;

      if (all_conds_ == bddfalse)
	{
	  // SUCCESSORS_ is 0 when step() is called from first().
	  if (successors_)
	    operand_->next();
	  all_conds_ = operand_->current_condition();
	  outside_ = !all_conds_;
	}

      bdd cond = bdd_satone(all_conds_);
      cond = bdd_simplify(cond, cond | outside_);
      all_conds_ -= cond;

      // Translate COND into an array of properties.
      signed char* props = data_->all_props;
      memset(props, -1, data_->prop_count);
      while (cond != bddtrue)
	{
	  int var = bdd_var(cond);

	  bdd high = bdd_high(cond);
	  int res;
	  if (high == bddfalse)
	    {
	      cond = bdd_low(cond);
	      res = 0;
	    }
	  else
	    {
	      cond = high;
	      res = 1;
	    }

	  // It's OK if VAR is unknown from GreatSPN (it might have
	  // been used to synchornize other automaton or something),
	  // just skip it.
	  tgba_gspn_eesrg_private_::prop_map::iterator i =
	    data_->prop_dict.find(var);
	  if (i != data_->prop_dict.end())
	    props[i->second] = res;

	  assert(cond != bddfalse);
	}

      succ(state_, props, &successors_, &size_);
    }

    virtual void
    first()
    {
      size_ = 0;
      all_conds_ = bddfalse;
      operand_->first();
      if (operand_->done())
	{
	  delete operand_;
	  operand_ = 0;
	  return;
	}
      if (successors_)
	{
	  delete successors_;
	  successors_ = 0;
	}
      step();
    }

    virtual void
    next()
    {
      assert(!done());
      step();
    }

    virtual bool
    done() const
    {
      return (size_ <= current_
	      && all_conds_ == bddfalse
	      && operand_
	      && operand_->done());
    }

    virtual state*
    current_state() const
    {
      return new state_gspn_eesrg(successors_[current_],
				  operand_->current_state());
    }

    virtual bdd
    current_condition() const
    {
      return bddtrue;
    }

    virtual bdd
    current_accepting_conditions() const
    {
      // There is no acceptance conditions in GSPN systems, so we just
      // return those from OPERAND_.
      return operand_->current_accepting_conditions();
    }
  private:
    State state_;
    // Iterator on the right operand
    tgba_succ_iterator* operand_;
    // All conditions of the current successor of the right operand
    // (We will iterate on all conjunctions in this.)
    bdd all_conds_;
    bdd outside_;
    // All successors of STATE matching a selection conjunctions from
    // ALL_CONDS.
    State* successors_;		/// array of successors
    size_t size_;		/// size of successors_
    size_t current_;		/// current position in successors_
    tgba_gspn_eesrg_private_* data_;
  }; // tgba_succ_iterator_gspn_eesrg


  // tgba_gspn_eesrg
  //////////////////////////////////////////////////////////////////////


  tgba_gspn_eesrg::tgba_gspn_eesrg(bdd_dict* dict, const gspn_environment& env,
				   const tgba* operand)
  {
    data_ = new tgba_gspn_eesrg_private_(dict, env, operand);
  }

  tgba_gspn_eesrg::tgba_gspn_eesrg(const tgba_gspn_eesrg& other)
    : tgba()
  {
    data_ = other.data_;
    ++data_->refs;
  }

  tgba_gspn_eesrg::~tgba_gspn_eesrg()
  {
    if (--data_->refs == 0)
      delete data_;
  }

  tgba_gspn_eesrg&
  tgba_gspn_eesrg::operator=(const tgba_gspn_eesrg& other)
  {
    if (&other == this)
      return *this;
    this->~tgba_gspn_eesrg();
    new (this) tgba_gspn_eesrg(other);
    return *this;
  }

  state* tgba_gspn_eesrg::get_init_state() const
  {
    // Use 0 as initial state for the EESRG side.  State 0 does not
    // exists, but when passed to succ() it will produce the list
    // of initial states.
    return new state_gspn_eesrg(0, data_->operand->get_init_state());
  }

  tgba_succ_iterator*
  tgba_gspn_eesrg::succ_iter(const state* state,
		       const state* global_state,
		       const tgba* global_automaton) const
  {
    const state_gspn_eesrg* s = dynamic_cast<const state_gspn_eesrg*>(state);
    assert(s);
    (void) global_state;
    (void) global_automaton;
    tgba_succ_iterator* i = data_->operand->succ_iter(s->right());
    return new tgba_succ_iterator_gspn_eesrg(s->left(), data_, i);
  }

  bdd
  tgba_gspn_eesrg::compute_support_conditions(const spot::state* state) const
  {
    (void) state;
    return bddtrue;
  }

  bdd
  tgba_gspn_eesrg::compute_support_variables(const spot::state* state) const
  {
    (void) state;
    return bddtrue;
  }

  bdd_dict*
  tgba_gspn_eesrg::get_dict() const
  {
    return data_->dict;
  }

  std::string
  tgba_gspn_eesrg::format_state(const state* state) const
  {
    const state_gspn_eesrg* s = dynamic_cast<const state_gspn_eesrg*>(state);
    assert(s);
    std::ostringstream os;
    char* str;
    State gs = s->left();
    if (gs)
      {
	int err = print_state(gs, &str);
	if (err)
	  throw gspn_exeption("print_state()", err);
      }
    else
      {
	str = strdup("-1");
      }

    // Rewrite all new lines as \\\n.
    const char* pos = str;
    while (*pos)
      {
	switch (*pos)
	  {
	    // Rewrite all new lines as \\n, and strip the last one.
	  case '\n':
	    if (pos[1])
	      os << "\\n";
	    break;
	  default:
	    os << *pos;
	  }
	++pos;
      }
    free(str);
    return os.str() + " * " + data_->operand->format_state(s->right());
  }

  bdd
  tgba_gspn_eesrg::all_accepting_conditions() const
  {
    // There is no accepting conditions in GSPN systems.
    return bddfalse;
  }

  bdd
  tgba_gspn_eesrg::neg_accepting_conditions() const
  {
    // There is no accepting conditions in GSPN systems.
    return bddtrue;
  }

}
