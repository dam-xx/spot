// Copyright (C) 2003, 2004  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#ifndef SPOT_TGBAALGOS_MAGIC_HH
# define SPOT_TGBAALGOS_MAGIC_HH

#include "tgba/tgba.hh"
#include "emptiness.hh"

namespace spot
{
  /// \brief Returns an emptiness checker on the spot::tgba automaton \a a. 
  /// During the visit of \a a, the returned checker stores explicitely all 
  /// the traversed states.
  ///
  /// \pre The automaton \a a must have at most one accepting condition (i.e.
  /// it is a TBA).
  ///
  /// The method \a check() of the returned checker can be called several times 
  /// (until it returns a null pointer) to enumerate all the visited accepting 
  /// paths. The implemented algorithm is the following. 
  ///
  /// \verbatim
  /// procedure check ()
  /// begin
  ///   call dfs_blue(s0);
  /// end;
  ///
  /// procedure dfs_blue (s)
  /// begin
  ///   s.color = blue;
  ///   for all t in post(s) do
  ///     if t.color == white then
  ///       call dfs_blue(t);
  ///     end if;
  ///     if (the edge (s,t) is accepting) then
  ///       target = s;
  ///       call dfs_red(t);
  ///     end if;
  ///   end for;
  /// end;
  ///
  /// procedure dfs_red(s)
  /// begin
  ///   s.color = red;
  ///   if s == target then
  ///     report cycle
  ///   end if;
  ///   for all t in post(s) do
  ///     if t.color != red then
  ///       call dfs_red(t);
  ///     end if;
  ///   end for;
  /// end;
  /// \endverbatim
  ///
  /// This algorithm is an adaptation to TBA of the one
  /// (which deals with accepting states) presented in
  ///
  /// \verbatim
  ///  Article{         courcoubertis.92.fmsd,
  ///    author        = {Costas Courcoubetis and Moshe Y. Vardi and Pierre 
  ///                    Wolper and Mihalis Yannakakis},
  ///    title         = {Memory-Efficient Algorithm for the Verification of
  ///                    Temporal Properties},
  ///    journal       = {Formal Methods in System Design},
  ///    pages         = {275--288},
  ///    year          = {1992},
  ///    volume        = {1}
  ///  }
  /// \endverbatim
  ///
  emptiness_check* explicit_magic_search(const tgba *a);

  /// \brief Returns an emptiness checker on the spot::tgba automaton \a a. 
  /// During the visit of \a a, the returned checker does not store explicitely
  /// the traversed states but uses the bit state hashing technic. However, the
  /// implemented algorithm is the same as the one of
  /// spot::explicit_magic_search.
  ///
  /// \pre The automaton \a a must have at most one accepting condition (i.e.
  /// it is a TBA).
  ///
  /// \sa spot::explicit_magic_search
  ///
  emptiness_check* bit_state_hashing_magic_search(const tgba *a, size_t size);

  /// \brief Returns an emptiness check on the spot::tgba automaton \a a. 
  /// During the visit of \a a, the returned checker stores explicitely all 
  /// the traversed states.
  ///
  /// \pre The automaton \a a must have at most one accepting condition (i.e.
  /// it is a TBA).
  ///  
  /// The method \a check() of the returned checker can be called several times 
  /// (until it returns a null pointer) to enumerate all the visited accepting 
  /// paths. The implemented algorithm is the following: 
  /// 
  /// \verbatim
  /// procedure check ()
  /// begin
  ///   weight = 0;
  ///   call dfs_blue(s0);
  /// end;
  ///  
  /// procedure dfs_blue (s)
  /// begin
  ///   s.color = cyan;
  ///   s.weight = weight;
  ///   for all t in post(s) do
  ///     if t.color == white then
  ///       if the edge (s,t) is accepting then
  ///         weight = weight + 1;
  ///       end if;
  ///       call dfs_blue(t);
  ///       if the edge (s,t) is accepting then
  ///         weight = weight - 1;
  ///       end if;
  ///     else if t.color == cyan and 
  ///             (the edge (s,t) is accepting or 
  ///              weight > t.weight) then
  ///       report cycle;
  ///     end if;
  ///     if the edge (s,t) is accepting then
  ///       call dfs_red(t);
  ///     end if;
  ///   end for;
  ///   s.color = blue;
  /// end;
  /// 
  /// procedure dfs_red(s)
  /// begin
  ///   if s.color == cyan then
  ///     report cycle;
  ///   end if;
  ///   s.color = red;
  ///   for all t in post(s) do
  ///     if t.color != red then
  ///       call dfs_red(t);
  ///     end if;
  ///   end for;
  /// end;
  /// \endverbatim
  /// 
  /// It is an adaptation to TBA (and a slight extension) of the one 
  /// presented in
  /// \verbatim
  ///  InProceedings{   schwoon.05.tacas,
  ///    author        = {Stephan Schwoon and Javier Esparza},
  ///    title         = {A Note on On-The-Fly Verification Algorithms},
  ///    booktitle     = {TACAS'05},
  ///    pages         = {},
  ///    year          = {2005},
  ///    volume        = {},
  ///    series        = {LNCS},
  ///    publisher     = {Springer-Verlag}
  ///  }
  /// \endverbatim
  ///
  /// The extention consists in the introduction of a weight associated
  /// to each state in the blue stack (the cyan states). The weight of a
  /// cyan state corresponds to the number of accepting arcs traversed to reach
  /// it from the initial state. Weights are used to detect accepting cycle in
  /// the blue dfs.
  ///
  emptiness_check* explicit_se05_search(const tgba *a);

  /// \brief Returns an emptiness checker on the spot::tgba automaton \a a. 
  /// During the visit of \a a, the returned checker does not store explicitely
  /// the traversed states but uses the bit state hashing technic. However, the
  /// implemented algorithm is the same as the one of
  /// spot::explicit_se05_search.
  ///
  /// \pre The automaton \a a must have at most one accepting condition (i.e.
  /// it is a TBA).
  ///
  /// \sa spot::explicit_se05_search
  ///
  emptiness_check* bit_state_hashing_se05_search(const tgba *a, size_t size);

}

#endif // SPOT_TGBAALGOS_MAGIC_HH
