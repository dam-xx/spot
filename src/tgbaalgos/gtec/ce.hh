// Copyright (C) 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#ifndef SPOT_TGBAALGOS_GTEC_CE_HH
# define SPOT_TGBAALGOS_GTEC_CE_HH

#include "status.hh"
#include "explscc.hh"

#include "tgbaalgos/minimalce.hh"

namespace spot
{
  /// Compute a counter example from a spot::emptiness_check_status
  class counter_example
  {
  public:
    counter_example(const emptiness_check_status* ecs,
		    const explicit_connected_component_factory*
		    eccf = connected_component_hash_set_factory::instance());

    typedef std::pair<const state*, bdd> state_proposition;
    typedef std::list<const state*> state_sequence;
    typedef std::list<state_proposition> cycle_path;
    state_sequence suffix;
    cycle_path period;

    /// \brief Display the example computed by counter_example().
    ///
    /// \param os the output stream
    /// \param restrict optional automaton to project the example on.
    std::ostream& print_result(std::ostream& os,
			       const tgba* restrict = 0) const;

    /// Output statistics about this object.
    void print_stats(std::ostream& os) const;

    ce::counter_example* get_counter_example() const;

  protected:
    /// Called by counter_example to find a path which traverses all
    /// acceptance conditions in the accepted SCC.
    void accepting_path (const explicit_connected_component* scc,
			 const state* start, bdd acc_to_traverse);

    /// Complete a cycle that caraterise the period of the counter
    /// example.  Append a sequence to the path given by accepting_path.
    void complete_cycle(const explicit_connected_component* scc,
			const state* from, const state* to);

  private:
    const emptiness_check_status* ecs_;
    ce::counter_example* counter_;
  };
}

#endif // SPOT_TGBAALGOS_GTEC_CE_HH
