// Copyright (C) 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#include "replayrun.hh"
#include "tgba/tgba.hh"
#include "emptiness.hh"
#include "tgba/bddprint.hh"

namespace spot
{
  bool
  replay_tgba_run(std::ostream& os, const tgba* a, const tgba_run* run)
  {
    const state* s = a->get_init_state();
    int serial = 1;
    const tgba_run::steps* l;
    std::string in;
    bdd all_acc = bddfalse;
    bdd expected_all_acc = a->all_acceptance_conditions();
    bool all_acc_seen = false;

    if (run->prefix.empty())
      {
	l = &run->cycle;
	in = "cycle";
      }
    else
      {
	l = &run->prefix;
	in = "prefix";
      }

    tgba_run::steps::const_iterator i = l->begin();

    if (s->compare(i->s))
      {
	os << "ERROR: First state of run (in " << in << "): "
	   << a->format_state(i->s) << std::endl
	   << "does not match initial state of automata: "
	   << a->format_state(s) << std::endl;
	delete s;
	return false;
      }

    for (; i != l->end(); ++serial)
      {
	os << "state " << serial << " in " << in << ": "
	   << a->format_state(s) << std::endl;

	// expected outgoing transition
	bdd label = i->label;
	bdd acc = i->acc;

	// compute the next expected state
	const state* next;
	++i;
	if (i != l->end())
	  {
	    next = i->s;
	  }
	else
	  {
	    if (l == &run->prefix)
	      {
		l = &run->cycle;
		in = "cycle";
		i = l->begin();
	      }
	    next = l->begin()->s;
	  }

	// browse the actual outgoing transitions
	tgba_succ_iterator* j = a->succ_iter(s);
	delete s;
	for (j->first(); !j->done(); j->next())
	  {
	    if (j->current_condition() != label
		|| j->current_acceptance_conditions() != acc)
	      continue;

	    const state* s2 = j->current_state();
	    if (s2->compare(next))
	      {
		delete s2;
		continue;
	      }
	    else
	      {
		s = s2;
		break;
	      }
	  }
	if (j->done())
	  {
	    os << "ERROR: no transition with label="
	       << bdd_format_formula(a->get_dict(), label)
	       << " and acc=" << bdd_format_accset(a->get_dict(), acc)
	       << " leaving state " << serial
	       << " for state " << a->format_state(next)
	       << std::endl
	       << "The following transitions leave state " << serial
	       << ":" << std::endl;
	    for (j->first(); !j->done(); j->next())
	      {
		const state* s2 = j->current_state();
		os << "  * "
		   << "label=" << bdd_format_formula(a->get_dict(),
						     j->current_condition())
		   << " and acc="
		   << bdd_format_accset(a->get_dict(),
					j->current_acceptance_conditions())
		   << " going to " << a->format_state(s2) << std::endl;
		delete s2;
	      }
	    delete j;
	    return false;
	  }
	os << "transition with label="
	   << bdd_format_formula(a->get_dict(), label)
	   << " and acc=" << bdd_format_accset(a->get_dict(), acc)
	   << std::endl;
	delete j;

	// Sum acceptance conditions.
	if (l == &run->cycle && i != l->begin())
	  {
	    all_acc |= acc;
	    if (!all_acc_seen && all_acc == expected_all_acc)
	      {
		all_acc_seen = true;
		os << "all acceptance conditions ("
		   << bdd_format_accset(a->get_dict(), all_acc)
		   << ") have been seen"
		   << std::endl;
	      }
	  }
      }
    delete s;
    if (all_acc != expected_all_acc)
      {
	os << "ERROR: The cycle's acceptance conditions ("
	   << bdd_format_accset(a->get_dict(), all_acc) << ") do not"
	   << "match those of the automata ("
	   << bdd_format_accset(a->get_dict(), expected_all_acc)
	   << std::endl;
	return false;
      }
    return true;
  }
}
