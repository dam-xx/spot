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

namespace spot
{

  /// \addtogroup ec_misc
  /// @{

  class ec_statistics
  {
  public :
    ec_statistics()
    : states_(0), transitions_(0), depth_(0), max_depth_(0)
    {
    }

    void
    set_states(int n)
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
    inc_depth()
    {
      ++depth_;
      if (depth_ > max_depth_)
	max_depth_ = depth_;
    }

    void
    dec_depth()
    {
      --depth_;
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

  private :
    unsigned states_; /// number of disctint visited states
    unsigned transitions_; /// number of visited transitions
    unsigned depth_; /// maximal depth of the stack(s)
    unsigned max_depth_; /// maximal depth of the stack(s)
  };

  /// Accepting Cycle Search Space statistics
  class acss_statistics
  {
  public:
    virtual int acss_states() const = 0;
  };

  /// @}
}

#endif // SPOT_TGBAALGOS_EMPTINESS_STATS_HH
