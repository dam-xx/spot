#ifndef SPOT_TGBAALGOS_MAGIC_HH
# define SPOT_TGBAALGOS_MAGIC_HH

#include <list>
#include <utility>
#include <ostream>
#include "tgba/tgbatba.hh"

namespace spot
{
  /// \brief Emptiness check on spot::tgba_tba_proxy automata using
  /// the Magic Search algorithm. 
  ///
  /// This algorithm comes from
  /// \verbatim
  /// @InProceedings{   godefroid.93.pstv,
  ///   author        = {Patrice Godefroid and Gerard .J. Holzmann},
  ///   title         = {On the verification of temporal properties},
  ///   booktitle     = {Proceedings of the 13th IFIP TC6/WG6.1 International
  ///                   Symposium on Protocol Specification, Testing, and
  ///                   Verification (PSTV'93)},
  ///   month         = {May},
  ///   editor        = {Andr{\'e} A. S. Danthine and Guy Leduc
  ///                    and Pierre Wolper},
  ///   address       = {Liege, Belgium},
  ///   pages         = {109--124},
  ///   publisher     = {North-Holland},
  ///   year          = {1993},
  ///   series        = {IFIP Transactions},
  ///   volume        = {C-16},
  ///   isbn          = {0-444-81648-8}
  /// }
  /// \endverbatim
  struct magic_search
  {
    /// Initialize the Magic Search algorithm on the automaton \a a.
    magic_search(const tgba_tba_proxy *a);
    ~magic_search();

    /// \brief Perform a Magic Search.
    /// 
    /// \return true iff the algorithm has found a new accepting
    ///    path.
    ///
    /// check() can be called several times until it return false,
    /// to enumerate all accepting paths.
    bool check();

    /// Print the last accepting path found.
    std::ostream& print_result(std::ostream& os) const;

  private:

    // The names "stack", "h", and "x", are those used in the paper.

    /// \brief  Records whether a state has be seen with the magic bit
    /// on or off.
    struct magic
    {
      bool seen_without : 1;
      bool seen_with    : 1;
    };
    
    /// \brief A state for the spot::magic_search algorithm.
    struct magic_state
    {
      const state* s;
      bool m;			///< The state of the magic demon.
    };

    typedef std::pair<magic_state, tgba_succ_iterator*> state_iter_pair;
    typedef std::list<state_iter_pair> stack_type;
    stack_type stack;		///< Stack of visited states on the path.

    typedef std::list<bdd> tstack_type;
    /// \brief Stack of transitions.  
    ///
    /// This is an addition to the data from the paper.
    tstack_type tstack;

    // FIXME: use a hash_map.
    typedef std::map<const state*, magic, state_ptr_less_than> hash_type;
    hash_type h;		///< Map of visited states.

    /// Append a new state to the current path.
    void push(const state* s, bool m);
    /// Check whether we already visited \a s with the Magic bit set to \a m.
    bool has(const state* s, bool m) const;

    const tgba_tba_proxy* a;	///< The automata to check.
    /// The state for which we are currently seeking an SCC.
    const state* x;		
  };


}

#endif // SPOT_TGBAALGOS_MAGIC_HH
