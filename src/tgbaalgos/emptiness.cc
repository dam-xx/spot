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

#include "emptiness.hh"
#include "tgba/tgba.hh"
#include "tgba/bddprint.hh"

namespace spot
{

  tgba_run::~tgba_run()
  {
    for (steps::const_iterator i = prefix.begin(); i != prefix.end(); ++i)
      delete i->s;
    for (steps::const_iterator i = cycle.begin(); i != cycle.end(); ++i)
      delete i->s;
  }

  tgba_run::tgba_run(const tgba_run& run)
  {
    for (steps::const_iterator i = run.prefix.begin();
	 i != run.prefix.end(); ++i)
      {
	step s = { s.s->clone(), i->label, i->acc };
	prefix.push_back(s);
      }
    for (steps::const_iterator i = run.cycle.begin();
	 i != run.cycle.end(); ++i)
      {
	step s = { s.s->clone(), i->label, i->acc };
	cycle.push_back(s);
      }
  }

  std::ostream& print_tgba_run(std::ostream& os,
			       const tgba_run* run,
			       const tgba* a)
  {
    bdd_dict* d = a->get_dict();
    os << "Prefix:" << std::endl;
    for (tgba_run::steps::const_iterator i = run->prefix.begin();
	 i != run->prefix.end(); ++i)
      {
	os << "  " << a->format_state(i->s) << std::endl;
	os << "  |  ";
	bdd_print_set(os, d, i->label);
	os << "\t";
	bdd_print_accset(os, d, i->acc);
	os << std::endl;
      }
    os << "Cycle:" << std::endl;
    for (tgba_run::steps::const_iterator i = run->cycle.begin();
	 i != run->cycle.end(); ++i)
      {
	os << "  " << a->format_state(i->s) << std::endl;
	os << "  |  ";
	bdd_print_set(os, d, i->label);
	os << "\t";
	bdd_print_accset(os, d, i->acc);
	os << std::endl;
      }
    return os;
  }


  tgba_run&
  tgba_run::operator=(const tgba_run& run)
  {
    if (&run != this)
      {
	this->~tgba_run();
	new(this) tgba_run(run);
      }
    return *this;
  }


  tgba_run*
  emptiness_check_result::accepting_run()
  {
    return 0;
  }

  emptiness_check::~emptiness_check()
  {
  }

  std::ostream&
  emptiness_check::print_stats(std::ostream& os) const
  {
    return os;
  }
}
