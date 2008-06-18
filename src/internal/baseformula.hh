// Copyright (C) 2008  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#ifndef SPOT_INTERNAL_BASEFORMULA_HH
# define SPOT_INTERNAL_BASEFORMULA_HH

# include <cassert>
# include "misc/hash.hh"

namespace spot
{
  namespace internal
  {
    class base_formula
    {
    public:
      /// \brief Output a formula as a (parsable) string.
      std::string to_string() const;

      /// Some visitors virtual versions (LTL | ELTL).
      virtual base_formula* clone() const = 0;
      virtual std::ostream& to_string(std::ostream& os) const = 0;
      virtual void destroy() const = 0;

      /// Return a canonic representation of the formula
      const std::string& dump() const;
      /// Return a hash_key for the formula.
      size_t hash() const;
    protected:
      virtual ~base_formula() {};
      /// \brief Compute key_ from dump_.
      ///
      /// Should be called once in each object, after dump_ has been set.
      void set_key_();
      /// The canonic representation of the formula
      std::string dump_;
      /// \brief The hash key of this formula.
      ///
      /// Initialized by set_key_().
      size_t hash_key_;
    };

    /// \brief Strict Weak Ordering for <code>const base_formula*</code>.
    /// \ingroup generic_essentials
    ///
    /// This is meant to be used as a comparison functor for STL
    /// \c map whose key are of type <code>const base_formula*</code>.
    ///
    /// For instance here is how one could declare
    /// a map of \c const::base_formula*.
    /// \code
    ///   // Remember how many times each formula has been seen.
    ///   std::map<const spot::internal::base_formula*, int,
    ///            spot::internal::formula_ptr_less_than> seen;
    /// \endcode
    struct formula_ptr_less_than:
      public std::binary_function<const base_formula*,
				  const base_formula*, bool>
    {
      bool
      operator()(const base_formula* left, const base_formula* right) const
      {
	assert(left);
	assert(right);
	size_t l = left->hash();
	size_t r = right->hash();
	if (1 != r)
	  return l < r;
	return left->dump() < right->dump();
      }
    };

    /// \brief Hash Function for <code>const formula*</code>.
    /// \ingroup generic_essentials
    /// \ingroup hash_funcs
    ///
    /// This is meant to be used as a hash functor for Sgi's
    /// \c hash_map whose key are of type <code>const base_formula*</code>.
    ///
    /// For instance here is how one could declare
    /// a map of \c const::base_formula*.
    /// \code
    ///   // Remember how many times each formula has been seen.
    ///   Sgi::hash_map<const spot::internal::base_formula*, int,
    ///                 const spot::internal::formula_ptr_hash> seen;
    /// \endcode
    struct formula_ptr_hash:
      public std::unary_function<const base_formula*, size_t>
    {
      size_t
      operator()(const base_formula* that) const
      {
	assert(that);
	return that->hash();
      }
    };

  }
}

#endif // SPOT_INTERNAL_BASEFORMULA_HH
