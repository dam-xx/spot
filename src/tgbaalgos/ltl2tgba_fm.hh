// Copyright (C) 2003, 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#ifndef SPOT_TGBA_LTL2TGBA_FME_HH
# define SPOT_TGBA_LTL2TGBA_FME_HH

#include "ltlast/formula.hh"
#include "tgba/tgbaexplicit.hh"

namespace spot
{
  /// \brief Build a spot::tgba_explicit* from an LTL formula.
  ///
  /// This is based on the following paper.
  /// \verbatim
  /// @InProceedings{couvreur.99.fm,
  ///   author	  = {Jean-Michel Couvreur},
  ///   title     = {On-the-fly Verification of Temporal Logic},
  ///   pages     = {253--271},
  ///   editor	  = {Jeannette M. Wing and Jim Woodcock and Jim Davies},
  ///   booktitle = {Proceedings of the World Congress on Formal Methods in the
  /// 		     Development of Computing Systems (FM'99)},
  ///   publisher = {Springer-Verlag},
  ///   series	  = {Lecture Notes in Computer Science},
  ///   volume	  = {1708},
  ///   year      = {1999},
  ///   address	  = {Toulouse, France},
  ///   month	  = {September},
  ///   isbn      = {3-540-66587-0}
  /// }
  /// \endverbatim
  ///
  /// If \a exprop is set, the algorithm will consider all properties
  /// combinations possible on each state, in an attempt to reduce
  /// the non-determinism.  The automaton will have the same size as
  /// without this option, but because the transition will be more
  /// deterministic, the product automaton will be smaller (or, at worse,
  /// equal).
  ///
  /// If \a symb_merge is set to false, states with the same symbolic
  /// representation (these are equivalent formulae) will not be
  /// merged.
  ///
  /// If \a branching_postponement is set, several transitions leaving
  /// from the same state with the same label (i.e., condition + acceptance
  /// conditions) will be merged.  This correspond to an optimization
  /// described in the following paper.
  /// \verbatim
  /// @InProceedings{	  sebastiani.03.charme,
  ///   author	  = {Roberto Sebastiani and Stefano Tonetta},
  ///   title	  = {"More Deterministic" vs. "Smaller" B{\"u}chi Automata for
  /// 		     Efficient LTL Model Checking},
  ///   booktitle = {Proceedings for the 12th Advanced Research Working
  /// 		     Conference on Correct Hardware Design and Verification
  /// 		     Methods (CHARME'03)},
  ///   pages     = {126--140},
  ///   year      = {2003},
  ///   editor	  = {G. Goos and J. Hartmanis and J. van Leeuwen},
  ///   volume	  = {2860},
  ///   series	  = {Lectures Notes in Computer Science},
  ///   month     = {October},
  ///   publisher = {Springer-Verlag}
  /// }
  /// \endverbatim
  tgba_explicit* ltl_to_tgba_fm(const ltl::formula* f, bdd_dict* dict,
				bool exprop = false, bool symb_merge = true,
				bool branching_postponement = false);
}

#endif // SPOT_TGBA_LTL2TGBA_HH
