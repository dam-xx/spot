#ifndef SPOT_TGBA_TGBABDDCONCRETEFACTORY_HH
# define SPOT_TGBA_TGBABDDCONCRETEFACTORY_HH

#include "ltlast/formula.hh"
#include "bddfactory.hh"
#include "tgbabddfactory.hh"

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

    /// Create a promise variable for formula \a f.
    ///
    /// \param f The formula to create a promise for.
    /// \return The variable number for this state.
    ///
    /// The promise is not created if it already exists.  Instead its
    /// existing variable number is returned.  Variable numbers
    /// can be turned into BDD using ithvar().
    int create_promise(const ltl::formula* f);

    const tgba_bdd_core_data& get_core_data() const;
    const tgba_bdd_dict& get_dict() const;

    /// Add a new constraint to the relation.
    void add_relation(bdd new_rel);

  private:
    tgba_bdd_core_data data_;	///< Core data for the new automata.
    tgba_bdd_dict dict_;	///< Dictionary for the new automata.
  };

}
#endif // SPOT_TGBA_TGBABDDCONCRETEFACTORY_HH
