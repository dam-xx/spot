#ifndef SPOT_IFACE_GSPN_HH
# define SPOT_IFACE_GSPN_HH

// Try not to include gspnlib.h here, or it will polute the user's
// namespace with internal C symbols.

# include <iostream>
# include <string>
# include "tgba/tgba.hh"
# include "ltlast/atomic_prop.hh"
# include "ltlenv/environment.hh"

namespace spot
{

  class gspn_interface
  {
  public:
    gspn_interface(int argc, char **argv);
    ~gspn_interface();
    tgba* get_automata();
  };


  /// An exeption used to forward GSPN errors.
  class gspn_exeption
  {
  public:
    gspn_exeption(const std::string& where, int err)
      : err_(err), where_(where)
    {
    }

    int
    get_err() const
    {
      return err_;
    }

    std::string
    get_where() const
    {
      return where_;
    }

  private:
    int err_;
    std::string where_;
  };

  std::ostream& operator<<(std::ostream& os, const gspn_exeption& e)
  {
    os << e.get_where() << " exited with " << e.get_err();
    return os;
  }

  class gspn_environment : public ltl::environment
  {
  public:
    gspn_environment();
    ~gspn_environment();

    /// Declare an atomic proposition.  Return false iff the
    /// proposition was already declared.
    bool declare(const std::string& prop_str);

    virtual ltl::formula* require(const std::string& prop_str);

    /// Get the name of the environment.
    virtual const std::string& name();

    typedef std::map<const std::string, ltl::atomic_prop*> prop_map;

    /// Get the map of atomic proposition known to this environment.
    const prop_map& get_prop_map() const;

  private:
    prop_map props_;
  };


  /// Data private to tgba_gspn.
  struct tgba_gspn_private_;

  class tgba_gspn: public tgba
  {
  public:
    tgba_gspn(bdd_dict* dict, const gspn_environment& env);
    tgba_gspn(const tgba_gspn& other);
    tgba_gspn& operator=(const tgba_gspn& other);
    virtual ~tgba_gspn();
    virtual state* get_init_state() const;
    virtual tgba_succ_iterator* succ_iter(const state* state) const;
    virtual bdd_dict* get_dict() const;
    virtual std::string format_state(const state* state) const;
    virtual bdd all_accepting_conditions() const;
    virtual bdd neg_accepting_conditions() const;
  private:
    tgba_gspn_private_* data_;
  };

}

#endif // SPOT_IFACE_GSPN_HH
