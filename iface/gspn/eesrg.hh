#ifndef SPOT_IFACE_GSPN_EESRG_HH
# define SPOT_IFACE_GSPN_EESRG_HH

// Do not include gspnlib.h here, or it will polute the user's
// namespace with internal C symbols.

# include <string>
# include "tgba/tgba.hh"
# include "common.hh"

namespace spot
{

  class gspn_eesrg_interface
  {
  public:
    gspn_eesrg_interface(int argc, char **argv);
    ~gspn_eesrg_interface();
    // FIXME: I think we should have
    // tgba* get_automata();
  };


  /// Data private to tgba_gspn.
  struct tgba_gspn_eesrg_private_;

  class tgba_gspn_eesrg: public tgba
  {
  public:
    tgba_gspn_eesrg(bdd_dict* dict, const gspn_environment& env,
		    const tgba* operand);
    tgba_gspn_eesrg(const tgba_gspn_eesrg& other);
    tgba_gspn_eesrg& operator=(const tgba_gspn_eesrg& other);
    virtual ~tgba_gspn_eesrg();
    virtual state* get_init_state() const;
    virtual tgba_succ_iterator*
    succ_iter(const state* local_state,
	      const state* global_state = 0,
	      const tgba* global_automaton = 0) const;
    virtual bdd_dict* get_dict() const;
    virtual std::string format_state(const state* state) const;
    virtual bdd all_accepting_conditions() const;
    virtual bdd neg_accepting_conditions() const;
  protected:
    virtual bdd compute_support_conditions(const spot::state* state) const;
    virtual bdd compute_support_variables(const spot::state* state) const;
  private:
    tgba_gspn_eesrg_private_* data_;
  };

}

#endif // SPOT_IFACE_GSPN_EESRG_GSPN_EESRG_HH
