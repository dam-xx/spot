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
    struct transition
    {
      bdd condition;
      bdd promise;
      state* dest;
    };

    transition*
    create_transition(const std::string& source, const std::string& dest);

    void add_condition(transition* t, ltl::formula* f);
    void add_promise(transition* t, ltl::formula* f);

    // tgba interface
    virtual ~tgba_explicit();
    virtual spot::state* get_init_state() const;
    virtual tgba_succ_iterator*
    succ_iter(const spot::state* state) const;
    virtual const tgba_bdd_dict& get_dict() const;
    virtual std::string format_state(const spot::state* state) const;

  protected:
    state* add_state(const std::string& name);
    typedef std::map<const std::string, tgba_explicit::state*> ns_map;
    typedef std::map<const tgba_explicit::state*, std::string> sn_map;
    ns_map name_state_map_;
    sn_map state_name_map_;
    tgba_bdd_dict dict_;
    tgba_explicit::state* init_;
  };


  class state_explicit : public spot::state
  {
  public:
    state_explicit(const tgba_explicit::state* s)
      : state_(s)
    {
    }

    virtual int compare(const spot::state* other) const;

    virtual ~state_explicit()
    {
    }

    const tgba_explicit::state* get_state() const;
  private:
    const tgba_explicit::state* state_;
  };


  class tgba_explicit_succ_iterator : public tgba_succ_iterator
  {
  public:
    tgba_explicit_succ_iterator(const tgba_explicit::state* s);

    virtual
    ~tgba_explicit_succ_iterator()
    {
    }

    virtual void first();
    virtual void next();
    virtual bool done();

    virtual state_explicit* current_state();
    virtual bdd current_condition();
    virtual bdd current_promise();

  private:
    const tgba_explicit::state* s_;
    tgba_explicit::state::const_iterator i_;
  };

}

#endif // SPOT_TGBA_TGBAEXPLICIT_HH
