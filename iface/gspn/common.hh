#ifndef SPOT_IFACE_GSPN_COMMON_HH
# define SPOT_IFACE_GSPN_COMMON_HH

# include <string>
# include <iostream>
# include "ltlast/atomic_prop.hh"
# include "ltlenv/environment.hh"


// Do not include gspnlib.h here, or it will polute the user's
// namespace with internal C symbols.

namespace spot
{

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

  std::ostream& operator<<(std::ostream& os, const gspn_exeption& e);

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

}

#endif // SPOT_IFACE_GSPN_COMMON_HH
