#ifndef SPOT_TGBA_TGBABDDTRANSLATEPROXY_HH
# define SPOT_TGBA_TGBABDDTRANSLATEPROXY_HH

#include "tgba.hh"

namespace spot
{
  /// \brief Proxy for a spot::tgba_succ_iterator_concrete that renumber
  /// BDD variables on the fly.
  class tgba_bdd_translate_proxy_succ_iterator: public tgba_succ_iterator
  {
  public:
    tgba_bdd_translate_proxy_succ_iterator(tgba_succ_iterator* it,
					   bddPair* rewrite);
    virtual ~tgba_bdd_translate_proxy_succ_iterator();

    // iteration
    void first();
    void next();
    bool done();

    // inspection
    state* current_state();
    bdd current_condition();
    bdd current_promise();
  protected:
    tgba_succ_iterator* iter_;
    bddPair* rewrite_;
  };


  /// \brief Proxy for a spot::tgba_bdd_concrete that renumber BDD variables
  /// on the fly.
  class tgba_bdd_translate_proxy: public tgba
  {
  public:
    /// \brief Constructor.
    /// \param from The spot::tgba to masquerade.
    /// \param to The new dictionary to use.
    tgba_bdd_translate_proxy(const tgba& from,
			     const tgba_bdd_dict& to);

    virtual ~tgba_bdd_translate_proxy();

    virtual state* get_init_state() const;

    virtual tgba_bdd_translate_proxy_succ_iterator*
    succ_iter(const state* state) const;

    virtual const tgba_bdd_dict& get_dict() const;

    virtual std::string format_state(const state* state) const;

  private:
    const tgba& from_;		///< The spot::tgba to masquerade.
    tgba_bdd_dict to_;		///< The new dictionar to use.
    bddPair* rewrite_to_;	///< The rewriting pair for from->to.
    bddPair* rewrite_from_;	///< The rewriting pair for to->from.
  };

}

#endif // SPOT_TGBA_TGBABDDTRANSLATEPROXY_HH
