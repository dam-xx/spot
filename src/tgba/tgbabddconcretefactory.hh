#ifndef SPOT_TGBA_TGBABDDCONCRETEFACTORY_HH
# define SPOT_TGBA_TGBABDDCONCRETEFACTORY_HH

#include "ltlast/formula.hh"
#include "bddfactory.hh"
#include "tgbabddfactory.hh"
#include <map>

namespace spot
{
  /// Helper class to build a spot::tgba_bdd_concrete object.
  class tgba_bdd_concrete_factory: public bdd_factory, public tgba_bdd_factory
  {
  public:
    virtual ~tgba_bdd_concrete_factory();

    /// Create a state variable for formula \a f.
    ///
    /// \param f The formula to create a state for.
    /// \return The variable number for this state.
    ///
    /// The state is not created if it already exists.  Instead its
    /// existing variable number is returned.  Variable numbers
    /// can be turned into BDD using ithvar().
    int create_state(const ltl::formula* f);

    /// Create an atomic proposition variable for formula \a f.
    ///
    /// \param f The formula to create an aotmic proposition for.
    /// \return The variable number for this state.
    ///
    /// The atomic proposition is not created if it already exists.
    /// Instead its existing variable number is returned.  Variable numbers
    /// can be turned into BDD using ithvar().
    int create_atomic_prop(const ltl::formula* f);

    /// Declare an accepting condition.
    ///
    /// Formula such as 'f U g' or 'F g' make the promise
    /// that 'g' will be fulfilled eventually.  So once
    /// one of this formula has been translated into a BDD,
    /// we use declare_accepting_condition() to associate
    /// all other states to the accepting set of 'g'.
    ///
    /// \param b a BDD indicating which variables are in the
    ///          accepting set
    /// \param a the formula associated
    void declare_accepting_condition(bdd b, const ltl::formula* a);

    const tgba_bdd_core_data& get_core_data() const;
    const tgba_bdd_dict& get_dict() const;

    /// Add a new constraint to the relation.
    void add_relation(bdd new_rel);

    /// \brief Perfom final computations before the relation can be used.
    ///
    /// This function should be called after all propositions, state,
    /// accepting conditions, and constraints have been declared, and
    /// before calling get_code_data() or get_dict().
    void finish();

  private:
    tgba_bdd_core_data data_;	///< Core data for the new automata.
    tgba_bdd_dict dict_;	///< Dictionary for the new automata.

    typedef std::map<const ltl::formula*, bdd> acc_map_;
    acc_map_ acc_;		///< BDD associated to each accepting condition
  };

}
#endif // SPOT_TGBA_TGBABDDCONCRETEFACTORY_HH
