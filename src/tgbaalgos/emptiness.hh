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

#ifndef SPOT_TGBAALGOS_EMPTINESS_HH
# define SPOT_TGBAALGOS_EMPTINESS_HH

#include <list>
#include <iosfwd>
#include <bdd.h>
#include "tgba/state.hh"

namespace spot
{

  /// An accepted run, for a tgba.
  struct tgba_run
  {
    struct step {
      const state* s;
      bdd label;
      bdd acc;
    };

    typedef std::list<step> steps;

    steps prefix;
    steps cycle;

    ~tgba_run();
    tgba_run()
    {
    };
    tgba_run(const tgba_run& run);
    tgba_run& operator=(const tgba_run& run);
  };

  class tgba;
  std::ostream& print_tgba_run(std::ostream& os,
			       const tgba_run* run,
			       const tgba* a);

  /// \brief The result of an emptiness check.
  ///
  /// Instances of these class should not last longer than the
  /// instances of emptiness_check that produced them as they
  /// may reference data internal to the check.
  class emptiness_check_result
  {
  public:
    /// \brief Return a run accepted by the automata passed to
    /// the emptiness check.
    ///
    /// This method might actually compute the acceptance run.  (Not
    /// all emptiness check algorithms actually produce a
    /// counter-example as a side-effect of checking emptiness, some
    /// need some post-processing.)
    ///
    /// This can also return 0 if the emptiness check algorithm
    /// cannot produce a counter example (that does not mean there
    /// is no counter-example; the mere existence of an instance of
    /// this class asserts the existence of a counter-example).
    virtual tgba_run* accepting_run();
  };

  /// Common interface to emptiness check algorithms.
  class emptiness_check
  {
  public:
    virtual ~emptiness_check();

    /// \brief Check whether the automaton contain an accepting run.
    ///
    /// Return 0 if the automaton accept no run.  Return an instance
    /// of emptiness_check_result otherwise.  This instance might
    /// allow to obtain one sample acceptance run.  The result has to
    /// be destroyed before the emptiness_check instance that
    /// generated it.
    ///
    /// Some emptiness_check algorithms may allow check() to be called
    /// several time, but generally you should not assume that.
    virtual emptiness_check_result* check() = 0;

    /// Print statistics, if any.
    virtual std::ostream& print_stats(std::ostream& os) const;
  };

}

#endif // SPOT_TGBAALGOS_EMPTINESS_HH
