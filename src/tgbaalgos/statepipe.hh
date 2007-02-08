// Copyright (C) 2007  Laboratoire d'Informatique de Paris 6 (LIP6),
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

#ifndef SPOT_TGBAALGOS_STATEPIPE_HH
# define SPOT_TGBAALGOS_STATEPIPE_HH

#include <unistd.h>

namespace spot {
  class state;
  class tgba;

  class state_pipe
  {
  public:
    state_pipe();
    ~state_pipe();
    bool write_state(const state* s);
    state* read_state(const tgba* a);
    void close_write_end();
    void close_read_end();

  private:
    int fd_[2];  /* 0=in, 1=out */
    char* in_buf_;
    size_t in_buf_size_;
    char* out_buf_;
    size_t out_buf_size_;
    size_t in_next_size_;
    bool in_next_size_read_;
  };

}

#endif // SPOT_TGBAALGOS_STATEPIPE_HH
