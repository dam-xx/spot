#ifndef SPOT_TGBA_TGBATBA_HH
# define SPOT_TGBA_TGBATBA_HH

#include "tgba.hh"
#include "misc/bddlt.hh"

namespace spot
{

  /// \brief Degeneralize a spot::tgba on the fly.
  ///
  /// This class acts as a proxy in front of a spot::tgba, that should
  /// be degeneralized on the fly.
  ///
  /// This automaton is a spot::tgba, but it will always have exactly
  /// one accepting condition.
  ///
  /// The degeneralization is done by synchronizing the input
  /// automaton with a "counter" automaton such as the one shown in
  /// "On-the-fly Verification of Linear Temporal Logic" (Jean-Michel
  /// Couveur, FME99).
  ///
  /// If the input automaton uses N accepting conditions, the output
  /// automaton can have at most max(N,1)+1 times more states and
  /// transitions.
  class tgba_tba_proxy : public tgba
  {
  public:
    tgba_tba_proxy(const tgba* a);

    virtual ~tgba_tba_proxy();

    virtual state* get_init_state() const;

    virtual tgba_succ_iterator*
    succ_iter(const state* local_state,
	      const state* global_state = 0,
	      const tgba* global_automaton = 0) const;

    virtual bdd_dict* get_dict() const;

    virtual std::string format_state(const state* state) const;

    virtual state* project_state(const state* s, const tgba* t) const;

    virtual bdd all_accepting_conditions() const;
    virtual bdd neg_accepting_conditions() const;

    bool state_is_accepting(const state* state) const;

  protected:
    virtual bdd compute_support_conditions(const state* state) const;
    virtual bdd compute_support_variables(const state* state) const;

  private:
    const tgba* a_;
    typedef std::map<bdd, bdd, bdd_less_than> cycle_map;
    cycle_map acc_cycle_;
    bdd the_accepting_cond_;
    // Disallow copy.
    tgba_tba_proxy(const tgba_tba_proxy&);
    tgba_tba_proxy& tgba_tba_proxy::operator=(const tgba_tba_proxy&);
  };

}
#endif // SPOT_TGBA_TGBATBA_HH
