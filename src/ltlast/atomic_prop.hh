// Copyright (C) 2003, 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#ifndef SPOT_LTLAST_ATOMIC_PROP_HH
# define SPOT_LTLAST_ATOMIC_PROP_HH

#include <string>
#include <iostream>
#include <map>
#include "refformula.hh"
#include "ltlenv/environment.hh"

namespace spot
{
  namespace ltl
  {

    /// Atomic propositions.
    class atomic_prop : public ref_formula
    {
    public:
      /// Build an atomic proposition with name \a name in
      /// environment \a env.
      static atomic_prop* instance(const std::string& name, environment& env);

      virtual void accept(visitor& visitor);
      virtual void accept(const_visitor& visitor) const;

      /// Get the name of the atomic proposition.
      const std::string& name() const;
      /// Get the environment of the atomic proposition.
      environment& env() const;

      /// Number of instantiated atomic propositions.  For debugging.
      static unsigned instance_count();
      /// List all instances of atomic propositions.  For debugging.
      static std::ostream& dump_instances(std::ostream& os);

    protected:
      atomic_prop(const std::string& name, environment& env);
      virtual ~atomic_prop();

      typedef std::pair<std::string, environment*> pair;
      typedef std::map<pair, atomic_prop*> map;
      static map instances;

    private:
      std::string name_;
      environment* env_;
    };

  }
}

#endif // SPOT_LTLAST_ATOMICPROP_HH