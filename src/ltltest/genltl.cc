// Copyright (C) 2010 Laboratoire de Recherche et Développement de
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

#include <cassert>
#include <iostream>
#include <sstream>
#include <set>
#include <string>
#include <cstdlib>
#include <cstring>
#include "ltlast/allnodes.hh"
#include "ltlvisit/tostring.hh"
#include "ltlenv/defaultenv.hh"

using namespace spot;
using namespace spot::ltl;

environment& env(default_environment::instance());



// The five first classes defined here come from the following paper:
//
// @InProceedings{cichon.09.depcos,
//   author = {Jacek Cicho{\'n} and Adam Czubak and Andrzej Jasi{\'n}ski},
//   title = {Minimal {B\"uchi} Automata for Certain Classes of {LTL} Formulas},
//   booktitle = {Proceedings of the Fourth International Conference on
//                Dependability of Computer Systems},
//   pages = {17--24},
//   year = 2009,
//   publisher = {IEEE Computer Society},
// }

void
syntax(char* prog)
{
  std::cerr << "Usage: "<< prog << " [-s] F N" << std::endl
	    << std::endl
	    << "-s output using Spin's syntax" << std::endl
	    << "F  specifies the familly of LTL formula to build" << std::endl
	    << "N  is the size parameter of the familly" << std::endl
	    << std::endl
	    << "Classes available (F):" << std::endl
	    << "  1: F(p1&F(p2&F(p3&...F(pn)))) & F(q1&F(q2&F(q3&...F(qn))))"
	    << std::endl
	    << "  2: p&X(p&X(p&...X(p)))) & X(q&F(q&F(q&...F(q))))"
	    << std::endl
	    << "  3: p&(Xp)&(XXp)&...(X...X(p)) & p&(Xq)&(XXq)&...(X...X(q))"
	    << std::endl
	    << "  4: GF(p1)&GF(p2)&...&GF(pn)"
	    << std::endl
	    << "  5: FG(p1)|GF(p2)|...|GF(pn)"
	    << std::endl;
  exit(2);
}

int
to_int(const char* s)
{
  char* endptr;
  int res = strtol(s, &endptr, 10);
  if (*endptr)
    {
      std::cerr << "Failed to parse `" << s << "' as an integer." << std::endl;
      exit(1);
    }
  return res;
}


// F(p_1 & F(p_2 & F(p_3 & ... F(p_n))))
formula* E_n(std::string name, int n)
{
  if (n <= 0)
    return constant::true_instance();

  formula* result = 0;

  for (; n > 0; --n)
    {
      std::ostringstream p;
      p << name << n;
      formula* f = env.require(p.str());
      if (result)
	result = multop::instance(multop::And, f, result);
      else
	result = f;
      result = unop::instance(unop::F, result);
    }
  return result;
}

// p & X(p & X(p & ... X(p)))
formula* phi_n(std::string name, int n)
{
  if (n <= 0)
    return constant::true_instance();

  formula* result = 0;
  formula* p = env.require(name);
  for (; n > 0; --n)
    {
      if (result)
	result =
	  multop::instance(multop::And, p->clone(),
			   unop::instance(unop::X, result));
      else
	result = p;
    }
  return result;
}

formula* N_n(std::string name, int n)
{
  return unop::instance(unop::F, phi_n(name, n));
}

// p & X(p) & XX(p) & XXX(p) & ... X^n(p)
formula* phi_prime_n(std::string name, int n)
{
  if (n <= 0)
    return constant::true_instance();

  formula* result = 0;
  formula* p = env.require(name);
  for (; n > 0; --n)
    {
      if (result)
	{
	  p = unop::instance(unop::X, p->clone());
	  result = multop::instance(multop::And, result, p);
	}
      else
	{
	  result = p;
	}
    }
  return result;
}

formula* N_prime_n(std::string name, int n)
{
  return unop::instance(unop::F, phi_prime_n(name, n));
}


// GF(p_1) | GF(p_2) | ... | GF(p_n)
formula* FG_n(std::string name, int n)
{
  if (n <= 0)
    return constant::true_instance();

  formula* result = 0;

  for (int i = 1; i <= n; ++i)
    {
      std::ostringstream p;
      p << name << i;
      formula* f = unop::instance(unop::G,
				  unop::instance(unop::F,
						 env.require(p.str())));

      if (result)
	result = multop::instance(multop::And, f, result);
      else
	result = f;
    }
  return result;
}

// FG(p_1) | FG(p_2) | ... | FG(p_n)
formula* GF_n(std::string name, int n)
{
  if (n <= 0)
    return constant::false_instance();

  formula* result = 0;

  for (int i = 1; i <= n; ++i)
    {
      std::ostringstream p;
      p << name << i;
      formula* f = unop::instance(unop::F,
				  unop::instance(unop::G,
						 env.require(p.str())));

      if (result)
	result = multop::instance(multop::Or, f, result);
      else
	result = f;
    }
  return result;
}

int
main(int argc, char** argv)
{
  bool spin_syntax = false;
  if (argc >= 2 && !strcmp(argv[1], "-s"))
    {
      spin_syntax = true;
      --argc;
      ++argv;
    }

  if (argc != 3)
    syntax(argv[0]);

  int f = to_int(argv[1]);
  int n = to_int(argv[2]);

  formula* res = 0;

  switch (f)
    {
    case 1:
      res = multop::instance(multop::And, E_n("p", n), E_n("q", n));
      break;
    case 2:
      res = multop::instance(multop::And, N_n("p", n), N_n("q", n));
      break;
    case 3:
      res = multop::instance(multop::And, N_prime_n("p", n), N_prime_n("q", n));
      break;
    case 4:
      res = FG_n("p", n);
      break;
    case 5:
      res = GF_n("p", n);
      break;
    default:
      std::cerr << "Unknown familly " << f << std::endl;
      break;
    }

  if (spin_syntax)
    to_spin_string(res, std::cout, true) << std::endl;
  else
    to_string(res, std::cout, true) << std::endl;

  res->destroy();

  return 0;
}
