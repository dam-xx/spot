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

#ifndef SPOT_INTERNAL_ENVIRONMENT_HH
# define SPOT_INTERNAL_ENVIRONMENT_HH

# include "formula.hh"
# include <string>

namespace spot
{
  namespace internal
  {
    /// \brief An environment that describes atomic propositions.
    /// \ingroup generic_essential
    template<typename T>
    class environment
    {
    public:
      /// \brief Obtain the formula associated to \a prop_str
      ///
      /// Usually \a prop_str, is the name of an atomic proposition,
      /// and spot::internal::require simply returns the associated
      /// spot::internal::atomic_prop<T>.
      ///
      /// Note this is not a \c const method.  Some environments will
      /// "create" the atomic proposition when requested.
      ///
      /// We return a spot::internal::formula<T> instead of an
      /// spot::internal::atomic_prop<T>, because this
      /// will allow nifty tricks (e.g., we could name formulae in an
      /// environment, and let the parser build a larger tree from
      /// these).
      ///
      /// \return 0 iff \a prop_str is not part of the environment,
      ///   or the associated spot::internal::formula<T> otherwise.
      virtual formula<T>* require(const std::string& prop_str) = 0;

      /// Get the name of the environment.
      virtual const std::string& name() = 0;

      virtual
      ~environment()
      {
      }

      // FIXME: More functions will be needed later, but
      // it's enough for now.
    };

  }
}

#endif // SPOT_INTERNAL_ENVIRONMENT_HH
