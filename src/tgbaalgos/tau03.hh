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

#ifndef SPOT_TGBAALGOS_TAU03_HH
# define SPOT_TGBAALGOS_TAU03_HH

namespace spot
{
  class tgba;
  class emptiness_check;

  /// \addtogroup emptiness_check_algorithms
  /// @{

  /// \brief Returns an emptiness checker on the spot::tgba automaton \a a.
  ///
  /// \pre The automaton \a a must have at least one accepting condition.
  ///
  /// During the visit of \a a, the returned checker stores explicitely all
  /// the traversed states. The implemented algorithm is the following:
  ///
  /// \verbatim
  /// procedure check ()
  /// begin
  ///   call dfs_blue(s0);
  /// end;
  ///
  /// procedure dfs_blue (s)
  /// begin
  ///   s.color = cyan;
  ///   s.acc = emptyset;
  ///   for all t in post(s) do
  ///     if t.color == white then
  ///       call dfs_blue(t);
  ///     end if;
  ///   end for;
  ///   for all t in post(s) do
  ///     let (s, l, a, t) be the edge from s to t;
  ///     if a U s.acc not included in t.acc then
  ///       target = s;
  ///       call dfs_red(t, a U s.acc);
  ///     end if;
  ///   end for;
  ///   if s.acc == all_acc then
  ///     report a cycle;
  ///   end if;
  ///   s.color = blue;
  /// end;
  ///
  /// procedure dfs_red(s, A)
  /// begin
  ///   s.acc = s.acc U A;
  ///   // The following test has been added to the origiginal algorithm to
  ///   // report a cycle as soon as possible (and to mimic the classic magic
  ///   // search.
  ///   if s == target and s.acc == all_acc then
  ///     report a cycle;
  ///   end if;
  ///   for all t in post(s) do
  ///     let (s, l, a, t) be the edge from s to t;
  ///     if t.color != white and A not included in t.acc then
  ///       call dfs_red(t, A);
  ///     end if;
  ///   end for;
  /// end;
  /// \endverbatim
  ///
  /// This algorithm is the one presented in
  ///
  /// \verbatim
  /// @techreport{HUT-TCS-A83,
  ///    address = {Espoo, Finland},
  ///    author = {Heikki Tauriainen},
  ///    institution = {Helsinki University of Technology, Laboratory for
  ///    Theoretical Computer Science},
  ///    month = {December},
  ///    number = {A83},
  ///    pages = {132},
  ///    title = {On Translating Linear Temporal Logic into Alternating and
  ///    Nondeterministic Automata},
  ///    type = {Research Report},
  ///    year = {2003},
  ///    url = {http://www.tcs.hut.fi/Publications/info/bibdb.HUT-TCS-A83.shtml}
  /// }
  /// \endverbatim
  ///
  emptiness_check* explicit_tau03_search(const tgba *a);

  /// @}
}

#endif // SPOT_TGBAALGOS_MAGIC_HH