// Copyright (C) 2004, 2005  Laboratoire d'Informatique de Paris 6 (LIP6),
// département Systèmes Répartis Coopératifs (SRC), Université Pierre
// et Marie Curie.
//
// This file is part of Spot, a model checking library.
//
// Spot is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Spot is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Spot; see the file COPYING.  If not, write to the Free
// Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.

#ifndef SPOT_TGBAALGOS_EMPTINESS_STATS_HH
# define SPOT_TGBAALGOS_EMPTINESS_STATS_HH

#include <cassert>

namespace spot
{

  /// \addtogroup emptiness_check_stats
  /// @{

  /// \brief Emptiness-check statistics
  ///
  /// Implementations of spot::emptiness_check may also implement
  /// this interface.  Try to dynamic_cast the spot::emptiness_check
  /// pointer to know whether these statistics are available.
  class ec_statistics
  {
  public :
    ec_statistics()
    : states_(0), transitions_(0), depth_(0), max_depth_(0)
    {
    }

    void
    set_states(unsigned n)
    {
      states_ = n;
    }

    void
    inc_states()
    {
      ++states_;
    }

    void
    inc_transitions()
    {
      ++transitions_;
    }

    void
    inc_depth(unsigned n = 1)
    {
      depth_ += n;
      if (depth_ > max_depth_)
	max_depth_ = depth_;
    }

    void
    dec_depth(unsigned n = 1)
    {
      assert(depth_ >= n);
      depth_ -= n;
    }

    int
    states() const
    {
      return states_;
    }

    int
    transitions() const
    {
      return transitions_;
    }

    int
    max_depth() const
    {
      return max_depth_;
    }

    int
    depth() const
    {
      return depth_;
    }

  private :
    unsigned states_;		/// number of disctint visited states
    unsigned transitions_;	/// number of visited transitions
    unsigned depth_;		/// maximal depth of the stack(s)
    unsigned max_depth_;	/// maximal depth of the stack(s)
  };

  /// \brief Accepting Cycle Search Space statistics
  ///
  /// Implementations of spot::emptiness_check_result may also implement
  /// this interface.  Try to dynamic_cast the spot::emptiness_check_result
  /// pointer to know whether these statistics are available.
  class acss_statistics
  {
  public:
    /// Number of states in the search space for the accepting cycle.
    virtual int acss_states() const = 0;
  };

  /// \brief Accepting Run Search statistics.
  ///
  /// Implementations of spot::emptiness_check_result may also implement
  /// this interface.  Try to dynamic_cast the spot::emptiness_check_result
  /// pointer to know whether these statistics are available.
  class ars_statistics
  {
  public:
    ars_statistics()
      : prefix_states_(0), cycle_states_(0)
    {
    }

    void
    inc_ars_prefix_states()
    {
      ++prefix_states_;
    }

    int
    ars_prefix_states() const
    {
      return prefix_states_;
    }

    void
    inc_ars_cycle_states()
    {
      ++cycle_states_;
    }

    int
    ars_cycle_states() const
    {
      return cycle_states_;
    }

  private:
    unsigned prefix_states_;	/// states visited to construct the prefix
    unsigned cycle_states_;	/// states visited to construct the cycle
  };

  /// @}
}

#endif // SPOT_TGBAALGOS_EMPTINESS_STATS_HH
