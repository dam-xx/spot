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

#include "tgba/state.hh"
#include "tgba/tgba.hh"
#include "statepipe.hh"
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <errno.h>
#include <fcntl.h>		/* fcntl, O_NONBLOCK */

namespace spot {

  state_pipe::state_pipe()
    : in_buf_(0), in_buf_size_(0),
      out_buf_(0), out_buf_size_(0),
      in_next_size_read_(false)
  {
    if (pipe(fd_) != 0)
      {
	perror("failed to create pipe");
	abort();
      }

    // Make the read end non-blocking.
    int fd_flags = fcntl(fd_[0], F_GETFL);
    if (fd_flags == -1)
      {
	perror("failed to read descriptor flags");
	abort();
      }
    if (fcntl(fd_[0], F_SETFL, fd_flags | O_NONBLOCK) == -1)
      {
	perror("failed to write descriptor flags");
	abort();
      }

    // Make the write end non-blocking.
    fd_flags = fcntl(fd_[1], F_GETFL);
    if (fd_flags == -1)
      {
	perror("failed to read descriptor flags");
	abort();
      }
    if (fcntl(fd_[1], F_SETFL, fd_flags | O_NONBLOCK) == -1)
      {
	perror("failed to write descriptor flags");
	abort();
      }
  }

  state_pipe::~state_pipe()
  {
    close_write_end();
    close_read_end();
  }

  bool
  state_pipe::write_state(const state* s)
  {
    size_t n;
    for (;;)
      {
	n = s->serialize(out_buf_ + sizeof n, out_buf_size_);
	if (n <= out_buf_size_)
	  break;
	out_buf_ = static_cast<char*>(realloc(out_buf_,
					      (out_buf_size_ = n) + sizeof n));
	if (out_buf_ == 0)
	  {
	    perror("realloc() failed");
	    abort();
	  }
      }
    memcpy(out_buf_, &n, sizeof n);
    // FIXME: pipe I/O is only atomic for size under PIPE_BUF.
    ssize_t ret = write(fd_[1], out_buf_, n + sizeof n);
    if (ret == -1 && errno == EAGAIN)
      return true;
    if (ret < 0 || static_cast<size_t>(ret) != n + sizeof n)
      {
	perror("write() failed");
	abort();
      }
    return false;
  }

  state*
  state_pipe::read_state(const tgba* a)
  {
    if (!in_next_size_read_)
      {
	int ret = read(fd_[0], &in_next_size_, sizeof in_next_size_);

	if (ret == -1 && errno == EAGAIN)
	  return 0;
	if (ret == 0) // Pipe close, suiciding.
	  abort();
	if (ret != sizeof in_next_size_)
	  {
	    perror("read() failed");
	    abort();
	  }

	in_next_size_read_ = true;
      }

    if (in_next_size_ > in_buf_size_)
      {
	in_buf_ =
	  static_cast<char*>(realloc(in_buf_,
				     in_buf_size_ = in_next_size_));
	if (!in_buf_)
	  {
	    perror("realloc() failed");
	    abort();
	  }
      }

    ssize_t ret = read(fd_[0], in_buf_, in_next_size_);
    if (ret == -1 && errno == EAGAIN)
      return 0;
    if (ret < 0 || static_cast<size_t>(ret) != in_next_size_)
      {
	printf("errno=%d\n", errno);
	perror("read() failed");
	abort();
      }

    in_next_size_read_ = false;
    state* s;
    size_t n = a->deserialize_state(in_buf_, in_next_size_, &s);
    assert((n == in_next_size_) && s);
    return s;
  }

  void
  state_pipe::close_write_end()
  {
    if (fd_[1] != -1)
      {
	close(fd_[1]);
	fd_[1] = -1;
      }
    if (out_buf_)
      {
	free(out_buf_);
	out_buf_ = 0;
      }
  }

  void
  state_pipe::close_read_end()
  {
    if (fd_[0] != -1)
      {
	close(fd_[0]);
	fd_[0] = -1;
      }
    if (in_buf_)
      {
	free(in_buf_);
	in_buf_ = 0;
      }
  }

}
