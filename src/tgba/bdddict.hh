#ifndef SPOT_TGBA_BDDDICT_HH
# define SPOT_TGBA_BDDDICT_HH

#include <map>
#include <list>
#include <set>
#include <iostream>
#include <bdd.h>
#include "ltlast/formula.hh"
#include "misc/bddalloc.hh"

namespace spot
{

  /// Map BDD variables to formulae.
  class bdd_dict: public bdd_allocator
  {
  public:

    bdd_dict();
    ~bdd_dict();

    /// Formula-to-BDD-variable maps.
    typedef std::map<const ltl::formula*, int> fv_map;
    /// BDD-variable-to-formula maps.
    typedef std::map<int, const ltl::formula*> vf_map;

    fv_map now_map;		///< Maps formulae to "Now" BDD variables
    vf_map now_formula_map;	///< Maps "Now" BDD variables to formulae
    fv_map var_map;		///< Maps atomic propisitions to BDD variables
    vf_map var_formula_map;     ///< Maps BDD variables to atomic propisitions
    fv_map acc_map;		///< Maps accepting conditions to BDD variables
    vf_map acc_formula_map;	///< Maps BDD variables to accepting conditions

    /// \brief Map Next variables to Now variables.
    ///
    /// Use with BuDDy's bdd_replace() function.
    bddPair* next_to_now;
    /// \brief Map Now variables to Next variables.
    ///
    /// Use with BuDDy's bdd_replace() function.
    bddPair* now_to_next;

    /// \brief Register an atomic proposition.
    ///
    /// Return (and maybe allocate) a BDD variable designating formula
    /// \a f.  The \a for_me argument should point to the object using
    /// this BDD variable, this is used for reference counting.  It is
    /// perfectly safe to call this function several time with the same
    /// arguments.
    ///
    /// \return The variable number.  Use bdd_ithvar() or bdd_nithvar()
    ///   to convert this to a BDD.
    int register_proposition(const ltl::formula* f, const void* for_me);

    /// \brief Register a couple of Now/Next variables
    ///
    /// Return (and maybe allocate) two BDD variables for a state
    /// associated to formula \a f.  The \a for_me argument should point
    /// to the object using this BDD variable, this is used for
    /// reference counting.  It is perfectly safe to call this
    /// function several time with the same arguments.
    ///
    /// \return The first variable number.  Add one to get the second
    /// variable.  Use bdd_ithvar() or bdd_nithvar() to convert this
    /// to a BDD.
    int register_state(const ltl::formula* f, const void* for_me);

    /// \brief Register an atomic proposition.
    ///
    /// Return (and maybe allocate) a BDD variable designating an
    /// accepting set associated to formula \a f.  The \a for_me
    /// argument should point to the object using this BDD variable,
    /// this is used for reference counting.  It is perfectly safe to
    /// call this function several time with the same arguments.
    ///
    /// \return The variable number.  Use bdd_ithvar() or bdd_nithvar()
    ///   to convert this to a BDD.
    int register_accepting_variable(const ltl::formula* f, const void* for_me);

    /// \brief Duplicate the variable usage of another object.
    ///
    /// This tells this dictionary that the \a for_me object
    /// will be using the same BDD variables as the \a from_other objects.
    /// This ensure that the variables won't be freed when \a from_other
    /// is deleted if \a from_other is still alive.
    void register_all_variables_of(const void* from_other, const void* for_me);

    /// \brief Release the variables used by object.
    ///
    /// Usually called in the destructor if \a me.
    void unregister_all_my_variables(const void* me);

    /// Check whether formula \a f has already been registered by \a by_me.
    bool is_registered(const ltl::formula* f, const void* by_me);

    /// \brief Dump all variables for debugging.
    /// \param os The output stream.
    std::ostream& dump(std::ostream& os) const;

    /// \brief Make sure the dictionary is empty.
    ///
    /// This will print diagnostics and abort if the dictionary
    /// is not empty.  Use for debugging.
    void assert_emptiness() const;

  protected:
    /// BDD-variable reference counts.
    typedef std::set<const void*> ref_set;
    typedef std::map<int, ref_set> vr_map;
    vr_map var_refs;

  private:
    // Disallow copy.
    bdd_dict(const bdd_dict& other);
    bdd_dict& operator=(const bdd_dict& other);
  };


}

#endif // SPOT_TGBA_BDDDICT_HH
