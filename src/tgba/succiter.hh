#ifndef SPOT_TGBA_SUCCITER_H
# define SPOT_TGBA_SUCCITER_H

#include "state.hh"

namespace spot
{

  /// \brief Iterate over the successors of a state.
  ///
  /// This class provides the basic functionalities required to
  /// iterate over the successors of a state, as well as querying
  /// transition labels.  Because transitions are never explicitely
  /// encoded, labels (conditions and promises) can only be queried
  /// while iterating over the successors.
  class tgba_succ_iterator
  {
  public:
    virtual
    ~tgba_succ_iterator()
    {
    }

    /// \name Iteration
    //@{

    /// \brief Position the iterator on the first successor (if any).
    ///
    /// This method can be called several times to make multiple
    /// passes over successors.  
    ///
    /// \warning One should always call \c done() to
    /// ensure there is a successor, even after \c first().  A
    /// common trap is to assume that there is at least one
    /// successor: this is wrong.
    virtual void first() = 0;

    /// \brief Jump to the next successor (if any).
    ///
    /// \warning Again, one should always call \c done() to ensure
    /// there is a successor.
    virtual void next() = 0;

    /// \brief Check whether the iteration is finished.
    ///
    /// This function should be called after any call to \c first()
    /// or \c next() and before any enquiry about the current state.
    ///
    /// The usual way to do this is with a \c for loop.
    /// \code
    ///    for (s->first(); !s->done(); s->next())
    ///      ...
    /// \endcode
    virtual bool done() = 0;

    //@}

    /// \name Inspection
    //@{

    /// \brief Get the state of the current successor.
    ///
    /// Note that the same state may occur at different points
    /// in the iteration.  These actually correspond to the same
    /// destination.  It just means there were several transitions,
    /// with different conditions or promises, leading to the
    /// same state.
    virtual state* current_state() = 0;
    /// \brief Get the condition on the transition leading to this successor.
    virtual bdd current_condition() = 0;
    /// \brief Get the promise on the transition leading to this successor.
    virtual bdd current_promise() = 0;

    //@}
  };

}


#endif // SPOT_TGBA_SUCCITER_H
