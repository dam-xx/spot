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


#include <sstream>
#include <algorithm>
#include "setdotdec.hh"

namespace spot
{
  namespace
  {
    unsigned index_of(std::vector<state_set*>& v,
                      const state* s)
    {
      for (unsigned i = 1; i < v.size(); ++i)
      {
        if ((v[i])->find(s) != (v[i])->end())
          return i;
      }
      // Black is the default color for nodes that does not belong to any set.
      return 0;
    }
  }

  set_dotty_decorator::set_dotty_decorator(std::vector<state_set*>* v)
    : v_(v)
  {
    assert(v->size() <= 5);
    colors_.push_back("black");
    colors_.push_back("blue");
    colors_.push_back("red");
    colors_.push_back("green");
    colors_.push_back("gold");
    colors_.push_back("pink");
  }

  set_dotty_decorator::~set_dotty_decorator()
  {
  }

  std::string
  set_dotty_decorator::state_decl(const tgba*, const state* s, int,
                                     tgba_succ_iterator*,
                                     const std::string& label)
  {
    std::ostringstream oss;
    unsigned i = index_of(*v_, s);
    oss << "[color=\"" << colors_[i] << "\", " <<"label=\""
        << label << "\"]";
    return oss.str();
  }

  std::string
  set_dotty_decorator::link_decl(const tgba*,
                                         const state*, int,
                                         const state*, int,
                                         const tgba_succ_iterator*,
                                         const std::string& label)
  {
    return "[label=\"" + label + "\"]";
  }
}
