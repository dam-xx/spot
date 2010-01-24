// Copyright (C) 2008 Laboratoire de Recherche et Développement
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


#ifndef SPOT_IFACE_NIPS_NIPS_HH
# define SPOT_IFACE_NIPS_NIPS_HH

// Do not include nipsvm.h here, or it will polute the user's
// namespace with internal C symbols.

# include <string>
# include "tgba/tgba.hh"
# include "common.hh"


// Fwd declarations.
typedef struct nipsvm_t nipsvm_t;
typedef struct t_bytecode nipsvm_bytecode_t;

namespace spot
{

  /// \brief An interface to provide a PROMELA front-end.
  ///
  /// This interface let to use a Promela model as a Büchi automata.
  /// It uses the NIPS library, which provied a virtual machine for
  /// the state-space exploration of a Promela model, therefore, models
  /// must be compiled with the NIPS compiler
  /// (http://wwwhome.cs.utwente.nl/~michaelw/nips/).
  ///
  /// With this interface, properties to check aren't defined with the Spot LTL
  /// representation, but in defining correctness claims (a monitor) in the
  /// Promela model (see chapter 4, The Spin Model Checker: Primer and
  /// reference manual, Gerard J.Holzmann).
  class nips_interface
  {
  public:
    nips_interface(bdd_dict* dict, const std::string& filename);
    ~nips_interface();
    bool has_monitor() const;
    tgba* automaton();
  private:
    bdd_dict* dict_;
    nipsvm_t* nipsvm_;
    nipsvm_bytecode_t* bytecode_;
  };
}

#endif	// SPOT_IFACE_NIPS_NIPS_HH
