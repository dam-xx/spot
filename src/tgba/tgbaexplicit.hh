#ifndef SPOT_TGBA_TGBAEXPLICIT_HH
# define SPOT_TGBA_TGBAEXPLICIT_HH

#include <list>
#include <map>
#include "tgba.hh"
#include "ltlast/formula.hh"
#include "bddfactory.hh"

namespace spot
{
  // Forward declarations.  See below.
  class state_explicit;
  class tgba_explicit_succ_iterator;

  /// Explicit representation of a spot::tgba.
  class tgba_explicit : public tgba, public bdd_factory
  {
  public:
    tgba_explicit();

    struct transition;
    typedef std::list<transition*> state;

    /// Explicit transitions (used by spot::tgba_explicit).
    struct transition
    {
      bdd condition;
      bdd accepting_conditions;
      state* dest;
    };

    transition*
    create_transition(const std::string& source, const std::string& dest);

    void add_condition(transition* t, ltl::formula* f);
    void add_neg_condition(transition* t, ltl::formula* f);
    void declare_accepting_condition(ltl::formula* f);
    bool has_accepting_condition(ltl::formula* f) const;
    void add_accepting_condition(transition* t, ltl::formula* f);

    // tgba interface
    virtual ~tgba_explicit();
    virtual spot::state* get_init_state() const;
    virtual tgba_succ_iterator*
    succ_iter(const spot::state* state) const;
    virtual const tgba_bdd_dict& get_dict() const;
    virtual std::string format_state(const spot::state* state) const;

    virtual bdd all_accepting_conditions() const;
    virtual bdd neg_accepting_conditions() const;

  protected:
    state* add_state(const std::string& name);
    bdd get_condition(ltl::formula* f);
    bdd get_accepting_condition(ltl::formula* f);

    typedef std::map<const std::string, tgba_explicit::state*> ns_map;
    typedef std::map<const tgba_explicit::state*, std::string> sn_map;
    ns_map name_state_map_;
    sn_map state_name_map_;
    tgba_bdd_dict dict_;
    tgba_explicit::state* init_;
    mutable bdd all_accepting_conditions_;
    bdd neg_accepting_conditions_;
    mutable bool all_accepting_conditions_computed_;
  };


  /// States used by spot::tgba_explicit.
  class state_explicit : public spot::state
  {
  public:
    state_explicit(const tgba_explicit::state* s)
      : state_(s)
    {
    }

    virtual int compare(const spot::state* other) const;

    virtual state_explicit* clone() const;

    virtual ~state_explicit()
    {
    }

    const tgba_explicit::state* get_state() const;
  private:
    const tgba_explicit::state* state_;
  };


  /// Successor iterators used by spot::tgba_explicit.
  class tgba_explicit_succ_iterator: public tgba_succ_iterator
  {
  public:
    tgba_explicit_succ_iterator(const tgba_explicit::state* s, bdd all_acc);

    virtual
    ~tgba_explicit_succ_iterator()
    {
    }

    virtual void first();
    virtual void next();
    virtual bool done() const;

    virtual state_explicit* current_state() const;
    virtual bdd current_condition() const;
    virtual bdd current_accepting_conditions() const;

  private:
    const tgba_explicit::state* s_;
    tgba_explicit::state::const_iterator i_;
    bdd all_accepting_conditions_;
  };

}

#endif // SPOT_TGBA_TGBAEXPLICIT_HH
