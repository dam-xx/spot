// Copyright (C) 2010, 2011 Laboratoire de Recherche et D�veloppement de
// l'Epita (LRDE).
// Copyright (C) 2005  Laboratoire d'Informatique de Paris 6 (LIP6),
// d�partement Syst�mes R�partis Coop�ratifs (SRC), Universit� Pierre
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

#ifndef SPOT_LTLVISIT_RANDOMLTL_HH
# define SPOT_LTLVISIT_RANDOMLTL_HH

#include "apcollect.hh"
#include <iosfwd>

namespace spot
{
  namespace ltl
  {

    /// \brief Base class for random formula generators
    /// \ingroup ltl_io
    class random_formula
    {
    public:
      random_formula(unsigned proba_size,
		     const atomic_prop_set* ap):
	proba_size_(proba_size), proba_(new op_proba[proba_size_]), ap_(ap)
      {
      }

      ~random_formula()
      {
	delete proba_;
      }

      /// Return the set of atomic proposition used to build formulae.
      const atomic_prop_set*
      ap() const
      {
	return ap_;
      }

      /// \brief Generate a formula of size \a n.
      ///
      /// It is possible to obtain formulae that are smaller than \a
      /// n, because some simple simplifications are performed by the
      /// AST.  (For instance the formula <code>a | a</code> is
      /// automatically reduced to <code>a</code> by spot::ltl::multop.)
      formula* generate(int n) const;

      /// \brief Print the priorities of each operator, constants,
      /// and atomic propositions.
      std::ostream& dump_priorities(std::ostream& os) const;

      /// \brief Update the priorities used to generate the formulae.
      ///
      /// The initial priorities are defined in each sub class as follows.
      ///
      /// These priorities can be altered using this function.
      /// \a options should be comma-separated list of KEY=VALUE
      /// assignments, using keys from the above list.
      /// For instance <code>"xor=0, F=3"</code> will prevent \c xor
      /// from being used, and will raise the relative probability of
      /// occurrences of the \c F operator.
      const char* parse_options(char* options);

    protected:
      void update_sums();

      struct op_proba
      {
	const char* name;
	int min_n;
	double proba;
	typedef formula* (*builder)(const random_formula* rl, int n);
	builder build;
	void setup(const char* name, int min_n, builder build);
      };
      unsigned proba_size_;
      op_proba* proba_;
      double total_1_;
      op_proba* proba_2_;
      double total_2_;
      double total_2_and_more_;
      const atomic_prop_set* ap_;
    };


    /// \brief Generate random LTL formulae.
    /// \ingroup ltl_io
    ///
    /// This class recursively construct LTL formulae of a given size.
    /// The formulae will use the use atomic propositions from the
    /// set of proposition passed to the constructor, in addition to the
    /// constant and all LTL operators supported by Spot.
    ///
    /// By default each operator has equal chance to be selected.
    /// Also, each atomic proposition has as much chance as each
    /// constant (i.e., true and false) to be picked.  This can be
    /// tuned using parse_options().
    class random_ltl: public random_formula
    {
    public:
      /// Create a random LTL generator using atomic propositions from \a ap.
      ///
      /// The default priorities are defined as follows:
      ///
      /// \verbatim
      /// ap      n
      /// false   1
      /// true    1
      /// not     1
      /// F       1
      /// G       1
      /// X       1
      /// equiv   1
      /// implies 1
      /// xor     1
      /// R       1
      /// U       1
      /// W       1
      /// M       1
      /// and     1
      /// or      1
      /// \endverbatim
      ///
      /// Where \c n is the number of atomic propositions in the
      /// set passed to the constructor.
      ///
      /// This means that each operator has equal chance to be
      /// selected.  Also, each atomic proposition has as much chance
      /// as each constant (i.e., true and false) to be picked.
      ///
      /// These priorities can be changed use the parse_options method.
      random_ltl(const atomic_prop_set* ap);
    };
  }
}


#endif // SPOT_LTLVIST_RANDOMLTL_HH
