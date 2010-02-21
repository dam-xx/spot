// Copyright (C) 2009, 2010 Laboratoire de Recherche et Développement
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

#ifndef SPOT_TGBAALGOS_SETDOTDEC_HH
# define SPOT_TGBAALGOS_SETDOTDEC_HH

#include <vector>
#include "dottydec.hh"
#include "misc/hash.hh"
#include "tgba/tgba.hh"

namespace spot
{
  typedef Sgi::hash_set<const spot::state*,
                        spot::state_ptr_hash, spot::state_ptr_equal> state_set;
  /// \brief Highlight sets of states on a tgba.
  /// \ingroup tgba_dotty
  ///
  /// An instance of this class can be passed to spot::dotty_reachable.
  class set_dotty_decorator: public dotty_decorator
  {
  public:
    set_dotty_decorator(std::vector<state_set*>* l);
    virtual ~set_dotty_decorator();

    virtual std::string state_decl(const tgba* a, const state* s, int n,
				   tgba_succ_iterator* si,
				   const std::string& label);
    virtual std::string link_decl(const tgba* a,
				  const state* in_s, int in,
				  const state* out_s, int out,
				  const tgba_succ_iterator* si,
				  const std::string& label);
  private:
    std::vector<state_set*>* v_;
    std::vector<std::string> colors_;
  };
}

#endif // SPOT_TGBAALGOS_SETDOTDEC_HH
