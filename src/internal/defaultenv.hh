// Copyright (C) 2003, 2004, 2005  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#ifndef SPOT_INTERNAL_DEFAULTENV_HH
# define SPOT_INTERNAL_DEFAULTENV_HH

#include "atomic_prop.hh"
#include "environment.hh"

namespace spot
{
  namespace internal
  {

    /// \brief A laxist environment.
    /// \ingroup generic_environment
    ///
    /// This environment recognizes all atomic propositions.
    ///
    /// This is a singleton.  Use default_environment<T>::instance()
    /// to obtain the instance.
    template<typename T>
    class default_environment : public environment<T>
    {
    public:
      virtual ~default_environment();
      virtual formula<T>* require(const std::string& prop_str);
      virtual const std::string& name();

      /// Get the sole instance of spot::internal::default_environment<T>.
      static default_environment<T>& instance();
    protected:
      default_environment();
    };

  }
}

#include "defaultenv.hxx"

#endif // SPOT_INTERNAL_DEFAULTENV_HH
