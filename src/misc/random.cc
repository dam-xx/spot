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

#include "random.hh"
#include <cstdlib>

namespace spot
{
  void
  srand(unsigned int seed)
  {
#if HAVE_SRAND48 && HAVE_DRAND48
    ::srand48(seed);
#else
    ::srand(seed);
#endif
  }

  double
  drand()
  {
#if HAVE_SRAND48 && HAVE_DRAND48
    return ::drand48();
#else
    double r = ::rand();
    return r / (1.0 + RAND_MAX);
#endif
  }

  int
  mrand(int max)
  {
    return static_cast<int>(max * drand());
  }

  int
  rrand(int min, int max)
  {
    return min + static_cast<int>((max - min + 1) * drand());
  }

}
