// Copyright (C) 2009 Laboratoire de Recherche et Developpement de
// l'Epita (LRDE).
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

#include "sccfilter.hh"
#include "tgba/tgbaexplicit.hh"
#include "reachiter.hh"
#include "tgbaalgos/scc.hh"
#include <sstream>

namespace spot
{
  namespace
  {
    class filter_iter: public tgba_reachable_iterator_depth_first
    {
    public:
      filter_iter(const tgba* a,
		  const scc_map& sm,
		  const std::vector<bool>& useless)
	: tgba_reachable_iterator_depth_first(a),
	  out_(new tgba_explicit_string(a->get_dict())),
	  sm_(sm),
	  useless_(useless)

      {
	out_->copy_acceptance_conditions_of(a);
      }

      tgba_explicit_string*
      result()
      {
	return out_;
      }

      bool
      want_state(const state* s) const
      {
	return !useless_[sm_.scc_of_state(s)];
      }

      void
      process_link(const state* in_s, int in,
		   const state* out_s, int out,
		   const tgba_succ_iterator* si)
      {
	std::ostringstream in_name;
	in_name << "(#" << in << ") " << this->automata_->format_state(in_s);
	std::ostringstream out_name;
	out_name << "(#" << out << ") " << this->automata_->format_state(out_s);

	tgba_explicit::transition* t =
	  out_->create_transition(in_name.str(), out_name.str());
	out_->add_conditions(t, si->current_condition());

	// Do not output any acceptance condition if either the source or
	// the destination state do not belong to an accepting state.
	if (sm_.accepting(sm_.scc_of_state(in_s))
	    && sm_.accepting(sm_.scc_of_state(out_s)))
	  out_->add_acceptance_conditions(t,
					  si->current_acceptance_conditions());
      }

    private:
      tgba_explicit_string* out_;
      const scc_map& sm_;
      const std::vector<bool>& useless_;
    };
  } // anonymous


  tgba* scc_filter(const tgba* aut)
  {
    scc_map sm(aut);
    sm.build_map();
    scc_stats ss = build_scc_stats(sm);

    filter_iter fi(aut, sm, ss.useless_scc_map);
    fi.run();
    return fi.result();
  }



}
