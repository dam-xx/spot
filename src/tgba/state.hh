#ifndef SPOT_TGBA_STATE_HH
# define SPOT_TGBA_STATE_HH

namespace spot
{
  class state
  {
  public:
    // Compares two states (that come from the same automaton).
    //
    // This method returns an integer less than, equal to, or greater
    // than zero if THIS is found, respectively, to be less than, equal
    // to, or greater than OTHER according to some implicit total order.
    //
    // This method should not be called to compare state from
    // different automata.
    virtual int compare(const state& other) const = 0;

    virtual ~state()
    {
    }
  };
}

#endif // SPOT_TGBA_STATE_HH
