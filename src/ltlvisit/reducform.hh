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

#ifndef SPOT_LTLVISIT_REDUCFORM_HH
# define SPOT_LTLVISIT_REDUCFORM_HH

#include "ltlast/formula.hh"
#include "ltlast/visitor.hh"

namespace spot
{
  namespace ltl
  {

    /// Options for spot::ltl::reduce.
    enum reduce_options
      {
	/// No reduction.
	Reduce_None = 0,
	/// Basic reductions.
	Reduce_Basics = 1,
	/// Somenzi & Bloem syntactic implication.
	Reduce_Syntactic_Implications = 2,
	/// Etessami & Holzmann eventuality and universality reductions.
	Reduce_Eventuality_And_Universality = 4,
	/// All reductions.
	Reduce_All = -1U
      };

    /// \brief Reduce a formula \a f.
    ///
    /// \param f the formula to reduce
    /// \param opt a conjonction of spot::ltl::reduce_options specifying
    //             which optimizations to apply.
    /// \return the reduced formula
    formula* reduce(const formula* f, int opt = Reduce_All);

    /// Implement basic rewriting.
    formula* basic_reduce_form(const formula* f);

    /// Detect easy case of implies.
    /// True if f1 < f2, false otherwise.
    bool inf_form(const formula* f1, const formula* f2);
    /// Detect easy case of implies.
    /// If n = 0, true if !f1 < f2, false otherwise.
    /// If n = 1, true if f1 < !f2, false otherwise.
    bool infneg_form(const formula* f1, const formula* f2, int n);

    /// \brief Check whether a formula is eventual.
    ///
    /// FIXME: Describe what eventual formulae are.  Cite paper.
    bool is_eventual(const formula* f);

    /// \brief Check whether a formula is universal.
    ///
    /// FIXME: Describe what universal formulae are.  Cite paper.
    bool is_universal(const formula* f);

    /// Whether a formula starts with GF.
    bool is_GF(const formula* f);
    /// Whether a formula starts with FG.
    bool is_FG(const formula* f);
  }
}

#endif //  SPOT_LTLVISIT_REDUCFORM_HH
