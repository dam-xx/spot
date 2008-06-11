// Copyright (C) 2003, 2004, 2005, 2008  Laboratoire d'Informatique de Paris 6 (LIP6),
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

/// \file internal/formula.hh
/// \brief Generic formula interface
#ifndef SPOT_INTERNAL_FORMULA_HH
# define SPOT_INTERNAL_FORMULA_HH

# include <string>
# include <cassert>
# include "predecl.hh"
# include "misc/hash.hh"

namespace spot
{
  namespace internal
  {
    /// \brief A Generic formula.
    /// \ingroup generic_essential
    /// \ingroup generic_ast
    ///
    /// The only way you can work with a formula is to
    /// build a T::visitor or T::const_visitor.
    template<typename T>
    class formula : public T
    {
    public:
      typedef typename T::visitor visitor;
      typedef typename T::const_visitor const_visitor;

      /// Entry point for T::visitor instances.
      virtual void accept(visitor& v) = 0;
      /// Entry point for T::const_visitor instances.
      virtual void accept(const_visitor& v) const = 0;

      /// \brief clone this node
      ///
      /// This increments the reference counter of this node (if one is
      /// used).  You should almost never use this method directly as
      /// it doesn't touch the children.  If you want to clone a
      /// whole formula, use a clone visitor instead.
      formula<T>* ref();
      /// \brief release this node
      ///
      /// This decrements the reference counter of this node (if one is
      /// used) and can free the object.  You should almost never use
      /// this method directly as it doesn't touch the children.  If you
      /// want to release a whole formula, use a destroy() visitor instead.
      static void unref(formula<T>* f);

      /// Return a canonic representation of the formula
      const std::string& dump() const;

      /// Return a hash_key for the formula.
      size_t
      hash() const
      {
	return hash_key_;
      }
    protected:
      virtual ~formula();

      /// \brief increment reference counter if any
      virtual void ref_();
      /// \brief decrement reference counter if any, return true when
      /// the instance must be deleted (usually when the counter hits 0).
      virtual bool unref_();

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

    /// \brief Strict Weak Ordering for <code>const formula*</code>.
    /// \ingroup generic_essentials
    ///
    /// This is meant to be used as a comparison functor for
    /// STL \c map whose key are of type <code>const formula<T>*</code>.
    ///
    /// For instance here is how one could declare
    /// a map of \c const::formula*.
    /// \code
    ///   // Remember how many times each formula has been seen.
    ///   template<typename T>
    ///   std::map<const spot::internal::formula<T>*, int,
    ///            spot::internal::formula_ptr_less_than<T> > seen;
    /// \endcode
    template<typename T>
    struct formula_ptr_less_than:
      public std::binary_function<const formula<T>*, const formula<T>*, bool>
    {
      bool
      operator()(const formula<T>* left, const formula<T>* right) const
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
    /// This is meant to be used as a hash functor for
    /// Sgi's \c hash_map whose key are of type <code>const formula*</code>.
    ///
    /// For instance here is how one could declare
    /// a map of \c const::formula*.
    /// \code
    ///   // Remember how many times each formula has been seen.
    ///   template<typename T>
    ///   Sgi::hash_map<const spot::internal::formula<T>*, int,
    ///                 const spot::internal::formula_ptr_hash<T> > seen;
    /// \endcode
    template<typename T>
    struct formula_ptr_hash:
      public std::unary_function<const formula<T>*, size_t>
    {
      size_t
      operator()(const formula<T>* that) const
      {
    	assert(that);
    	return that->hash();
      }
    };

  }
}

# include "formula.hxx"

#endif // SPOT_INTERNAL_FORMULA_HH
