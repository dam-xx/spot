#ifndef SPOT_TGBA_STATE_HH
# define SPOT_TGBA_STATE_HH

#include <bdd.h>

namespace spot
{

  /// \brief Abstract class for states.
  class state
  {
  public:
    /// \brief Compares two states (that come from the same automaton).
    ///
    /// This method returns an integer less than, equal to, or greater
    /// than zero if \a this is found, respectively, to be less than, equal
    /// to, or greater than \a other according to some implicit total order.
    ///
    /// This method should not be called to compare states from
    /// different automata.
    ///
    /// \sa spot::state_ptr_less_than
    virtual int compare(const state* other) const = 0;

    virtual ~state()
    {
    }
  };

  /// \brief Strict Weak Ordering for \c state*.
  ///
  /// This is meant to be used as a comparison functor for
  /// STL \c map whose key are of type \c state*.
  ///
  /// For instance here is how one could declare
  /// a map of \c state*.
  /// \code
  ///   // Remember how many times each state has been visited.
  ///   std::map<spot::state*, int, spot::state_ptr_less_than> seen;
  /// \endcode
  struct state_ptr_less_than
  {
    bool
    operator()(const state* left, const state *right)
    {
      return left->compare(right) < 0;
    }
  };

}

#endif // SPOT_TGBA_STATE_HH
