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

#ifndef SPOT_TGBAALGOS_RANDOMGRAPH_HH
# define SPOT_TGBAALGOS_RANDOMGRAPH_HH

#include "ltlvisit/apcollect.hh"
#include "ltlenv/defaultenv.hh"

namespace spot
{
  class bdd_dict;
  class tgba;

  /// \brief Construct a tgba randomly.
  /// \ingroup tgba_misc
  ///
  /// \param n The number of states wanted in the automata.  All states
  ///          will be connected, and there will be no dead state.
  /// \param d The density of the automata.  This is the probability
  ///          (between 0.0 and 1.0), to add a transition between two
  ///          states.  All states have at least one outgoing transition,
  ///          so \a d is considered only when adding the remaining transition.
  ///          A density of 1 means all states will be connected to each other.
  /// \param ap The list of atomic property that should label the transition.
  /// \param dict The bdd_dict to used for this automata.
  /// \param n_acc The number of acceptance sets to use.
  /// \param a The probability (between 0.0 and 1.0) that a transition belongs
  ///          to an acceptance set.
  /// \param t The probability (between 0.0 and 1.0) that an atomic proposition
  ///          is true.
  /// \param env The environment in which to declare the acceptance conditions.
  ///
  /// This algorithms is adapted from the one in Fig 6.2 page 48 of
  /// \verbatim
  /// @TechReport{	  tauriainen.00.a66,
  ///   author	= {Heikki Tauriainen},
  ///   title   = {Automated Testing of {B\"u}chi Automata Translators for
  /// 		  {L}inear {T}emporal {L}ogic},
  ///   address	= {Espoo, Finland},
  ///   institution = {Helsinki University of Technology, Laboratory for
  /// 		  Theoretical Computer Science},
  ///   number	= {A66},
  ///   year	= {2000},
  ///   url	= {http://citeseer.nj.nec.com/tauriainen00automated.html},
  ///   type	= {Research Report},
  ///   note	= {Reprint of Master's thesis}
  /// }
  /// \endverbatim
  /// The only difference is that labels are on transitions, and that
  /// acceptance conditions are handled.  Otherwise the logic is the same.
  tgba*
  random_graph(int n, float d,
	       const ltl::atomic_prop_set* ap, bdd_dict* dict,
	       int n_acc = 0, float a = 0.1, float t = 0.5,
	       ltl::environment* env = &ltl::default_environment::instance());
}

#endif // SPOT_TGBAALGOS_RANDOMGRAPH_HH
