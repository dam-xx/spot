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

#ifndef SPOT_TGBAALGOS_EMPTINESS_HH
# define SPOT_TGBAALGOS_EMPTINESS_HH

#include <list>
#include <iosfwd>
#include <bdd.h>
#include "tgba/state.hh"
#include "emptiness_stats.hh"

namespace spot
{
  class tgba;
  struct tgba_run;

  /// \addtogroup emptiness_check Emptiness-checks
  /// \ingroup tgba_algorithms
  ///
  /// All emptiness-check algorithms follow the same interface.
  /// Basically once you have constructed an instance of
  /// spot::emptiness_check (by instantiating a subclass, or calling a
  /// functions construct such instance; see \ref
  /// emptiness_check_algorithms "this list"), you should call
  /// spot::emptiness_check::check() to check the automaton.
  ///
  /// If spot::emptiness_check::check() returns 0, then the automaton
  /// was found empty.  Otherwise the automaton accepts some run.
  /// (Beware that some algorithms---those using bit-state
  /// hashing---may found the automaton to be empty even if it is not
  /// actually empty.)
  ///
  /// When spot::emptiness_check::check() does not return 0, it
  /// returns an instance of spot::emptiness_check_result.  You can
  /// try to call spot::emptiness_check_result::accepting_run() to
  /// obtain an accepting run.  For some emptiness-check algorithms,
  /// spot::emptiness_check_result::accepting_run() will require some
  /// extra computation.  Most emptiness-check algorithms are able to
  /// return such an accepting run, however this is not mandatory and
  /// spot::emptiness_check_result::accepting_run() can return 0 (this
  /// does not means by anyway that no accepting run exist).
  ///
  /// The acceptance run returned by
  /// spot::emptiness_check_result::accepting_run(), if any, is of
  /// type spot::tgba_run.  \ref tgba_run "This page" gathers existing
  /// operations on these objects.
  ///
  /// @{

  /// \brief The result of an emptiness check.
  ///
  /// Instances of these class should not last longer than the
  /// instances of emptiness_check that produced them as they
  /// may reference data internal to the check.
  class emptiness_check_result
  {
  public:
    emptiness_check_result(const tgba* a)
      : a_(a)
    {
    }

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

    /// The automaton on which an accepting_run() was found.
    const tgba*
    automaton() const
    {
      return a_;
    }

    /// Return statistics, if available.
    virtual const unsigned_statistics* statistics() const;

  protected:
    const tgba* a_;		///< The automaton.
  };

  /// Common interface to emptiness check algorithms.
  class emptiness_check
  {
  public:
    emptiness_check(const tgba* a)
      : a_(a)
    {
    }
    virtual ~emptiness_check();

    /// The automaton that this emptiness-check inspects.
    const tgba*
    automaton() const
    {
      return a_;
    }

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

    /// Return statistics, if available.
    virtual const unsigned_statistics* statistics() const;

    /// Print statistics, if any.
    virtual std::ostream& print_stats(std::ostream& os) const;

  protected:
    const tgba* a_;		///< The automaton.
  };

  /// @}

  /// \addtogroup emptiness_check_algorithms Emptiness-check algorithms
  /// \ingroup emptiness_check


  /// \addtogroup tgba_run TGBA runs and supporting functions
  /// \ingroup emptiness_check
  /// @{

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

  /// \brief Display a tgba_run.
  ///
  /// Output the prefix and cycle of the tgba_run \a run, even if it
  /// does not corresponds to an actual run of the automaton \a a.
  /// This is unlike replay_tgba_run(), which will ensure the run
  /// actually exist in the automaton (and will display any transition
  /// annotation).
  ///
  /// (\a a is used here only to format states and transitions.)
  ///
  /// Output the prefix and cycle of the tgba_run \a run, even if it
  /// does not corresponds to an actual run of the automaton \a a.
  /// This is unlike replay_tgba_run(), which will ensure the run
  /// actually exist in the automaton (and will display any transition
  /// annotation).
  std::ostream& print_tgba_run(std::ostream& os,
                               const tgba* a,
                               const tgba_run* run);

  /// \brief Return an explicit_tgba corresponding to \a run (i.e. comparable
  /// states are merged).
  ///
  /// \pre \a run must correspond to an actual run of the automaton \a a.
  tgba* tgba_run_to_tgba(const tgba* a, const tgba_run* run);

  /// @}

  /// \addtogroup emptiness_check_stats Emptiness-check statistics
  /// \ingroup emptiness_check
}

#endif // SPOT_TGBAALGOS_EMPTINESS_HH
