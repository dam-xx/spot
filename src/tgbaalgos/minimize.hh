// Copyright (C) 2009, 2010, 2011 Laboratoire de Recherche et Développement
// de l'Epita (LRDE).
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

#ifndef SPOT_TGBAALGOS_MINIMIZE_HH
# define SPOT_TGBAALGOS_MINIMIZE_HH

# include "tgba/tgbaexplicit.hh"
# include "ltlast/formula.hh"

namespace spot
{
  /// \brief Use the powerset construction to minimize a TGBA.
  ///
  /// If \a monitor is set to \c false (the default), then the
  /// minimized automaton is correct only for properties that belong
  /// to the class of "obligation properties".  This algorithm assumes
  /// that the given automaton expresses an obligation properties and
  /// will return an automaton that is bogus (i.e. not equivalent to
  /// the original) if that is not the case.
  ///
  /// Please see the following paper for a discussion of this
  /// technique.
  ///
  /// \verbatim
  /// @InProceedings{	  dax.07.atva,
  ///   author	= {Christian Dax and Jochen Eisinger and Felix Klaedtke},
  ///   title		= {Mechanizing the Powerset Construction for Restricted
  /// 		  Classes of {$\omega$}-Automata},
  ///   year		= 2007,
  ///   series	= {Lecture Notes in Computer Science},
  ///   publisher	= {Springer-Verlag},
  ///   volume	= 4762,
  ///   booktitle	= {Proceedings of the 5th International Symposium on
  /// 		  Automated Technology for Verification and Analysis
  /// 		  (ATVA'07)},
  ///   editor	= {Kedar S. Namjoshi and Tomohiro Yoneda and Teruo Higashino
  /// 		  and Yoshio Okamura},
  ///   month		= oct
  /// }
  /// \endverbatim
  ///
  /// Dax et al. suggest one way to check whether a property
  /// \f$\varphi\f$ expressed as an LTL formula is an obligation:
  /// translate the formula and its negation as two automata \f$A_f\f$
  /// and \f$A_{\lnot f}\f$, then minimize both automata and check
  /// that the two products $\f \mathrm{minimize(A_{\lnot f})\otimes
  /// A_f\f$ and $\f \mathrm{minimize(A_f)\otimes A_{\lnot f}\f$ are
  /// empty.  If that is the case, then the minimization was correct.
  ///
  /// You may also want to check if \$A_f\$ is a safety automaton
  /// using the is_safety_automaton() function.  Since safety
  /// properties are a subclass of obligation properties, you can
  /// apply the minimization without further test.  Note however that
  /// this is only a sufficient condition.
  ///
  /// If \a monitor is set to \c true, the automaton will be converted
  /// into minimal deterministic monitor.  All useless SCCs should
  /// have been previously removed (using scc_filter() for instance).
  /// Then the automaton will be reduced as if all states where
  /// accepting states.
  ///
  /// For more detail about monitors, see the following paper:
  /// \verbatim
  /// @InProceedings{	  tabakov.10.rv,
  ///   author	= {Deian Tabakov and Moshe Y. Vardi},
  ///   title		= {Optimized Temporal Monitors for SystemC{$^*$}},
  ///   booktitle	= {Proceedings of the 10th International Conferance
  ///			   on Runtime Verification},
  ///   pages		= {436--451},
  ///   year		= 2010,
  ///   volume	= {6418},
  ///   series	= {Lecture Notes in Computer Science},
  ///   month		= nov,
  ///   publisher	= {Spring-Verlag}
  /// }
  /// \endverbatim
  /// (Note: although the above paper uses Spot, this function did not
  /// exist at that time.)
  tgba_explicit_number* minimize(const tgba* a, bool monitor = false);


  /// \brief Minimize an automaton if it represents an obligation property.
  ///
  /// This function attempt to minimize the automaton \a aut_f using the
  /// algorithm implemented in the minimize() function, and presented
  /// by the following paper:
  ///
  /// \verbatim
  /// @InProceedings{	  dax.07.atva,
  ///   author	= {Christian Dax and Jochen Eisinger and Felix Klaedtke},
  ///   title		= {Mechanizing the Powerset Construction for Restricted
  /// 		  Classes of {$\omega$}-Automata},
  ///   year		= 2007,
  ///   series	= {Lecture Notes in Computer Science},
  ///   publisher	= {Springer-Verlag},
  ///   volume	= 4762,
  ///   booktitle	= {Proceedings of the 5th International Symposium on
  /// 		  Automated Technology for Verification and Analysis
  /// 		  (ATVA'07)},
  ///   editor	= {Kedar S. Namjoshi and Tomohiro Yoneda and Teruo Higashino
  /// 		  and Yoshio Okamura},
  ///   month		= oct
  /// }
  /// \endverbatim
  ///
  /// Because it is hard to determine if an automaton correspond
  /// to an obligation property, you should supply either the formula
  /// \a f expressed by the automaton \a aut_f, or \a aut_neg_f the negation
  /// of the automaton \a aut_neg_f.
  ///
  /// \param aut_f the automaton to minimize
  /// \param f the LTL formula represented by the automaton \a aut_f
  /// \param aut_neg_f an automaton representing the negation of \a aut_f
  /// \return a new tgba if the automaton could be minimized, aut_f if
  /// the automaton cannot be minimized, 0 if we do not if if the
  /// minimization is correct because neither \a f nor \a aut_neg_f
  /// were supplied.
  ///
  /// The function proceeds as follows.  If the formula \a f or the
  /// automaton \a aut can easily be proved to represent an obligation
  /// formula, then the result of \code minimize(aut) is returned.
  /// Otherwise, if \a aut_neg_f was not supplied but \a f was, \a
  /// aut_neg_f is built from the negation of \a f.  Then we check
  /// that \code product(aut,!minimize(aut_f)) and \code
  /// product(aut_neg_f,minize(aut)) are both empty.  If they are, the
  /// the minimization was sound.  (See the paper for full details.)
  const tgba* minimize_obligation(const tgba* aut_f,
				  const ltl::formula* f = 0,
				  const tgba* aut_neg_f = 0);

}

#endif /* !SPOT_TGBAALGOS_MINIMIZE_HH */
