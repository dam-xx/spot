#include "minato.hh"
#include <utility>
#include <cassert>

namespace spot
{

  minato_isop::minato_isop(bdd input)
    : ret(bddfalse)
  {
    todo.push(local_vars(input, input));
    cube.push(bddtrue);
  }

  bdd
  minato_isop::next()
  {
    while (todo.size())
      {
	local_vars& l = todo.top();
	switch (l.step)
	  {
	  case local_vars::FirstStep:
	    if (l.f_min == bddfalse)
	      {
		ret = bddfalse;
		todo.pop();
		continue;
	      }
	    if (l.f_max == bddtrue)
	      {
		ret = bddtrue;
		todo.pop();
		return cube.top();
	      }
	    l.step = local_vars::SecondStep;

	    {
	      // Pick the topmost variable.
	      int v_min = bdd_var(l.f_min);
	      int v_max = bdd_var(l.f_max);
	      int v = std::min(v_min, v_max);

	      // The following two `if's do
	      //   v0 = bdd_nithvar(v);
	      //   v1 = bdd_ithvar(v);
	      //   f0_min = bdd_restrict(f_min, v0);
	      //   f0_max = bdd_restrict(f_max, v0);
	      //   f1_min = bdd_restrict(f_min, v1);
	      //   f1_max = bdd_restrict(f_max, v1);
	      // but since we now the v is the topmost variable,
	      // its more efficient to use bdd_low and bdd_high.
	      if (v == v_min)
		{
		  l.f0_min = bdd_low(l.f_min);
		  l.f1_min = bdd_high(l.f_min);
		}
	      else
		{
		  l.f1_min = l.f0_min = l.f_min;
		}
	      if (v == v_max)
		{
		  l.f0_max = bdd_low(l.f_max);
		  l.f1_max = bdd_high(l.f_max);
		}
	      else
		{
		  l.f1_max = l.f0_max = l.f_max;
		}

	      l.v1 = bdd_ithvar(v);
	      cube.push(cube.top() & bdd_nithvar(v));
	      todo.push(local_vars(l.f0_min - l.f1_max, l.f0_max));
	    }
	    continue;

	  case local_vars::SecondStep:
	    l.step = local_vars::ThirdStep;
	    l.g0 = ret;
	    cube.pop();
	    cube.push(cube.top() & l.v1);
	    todo.push(local_vars(l.f1_min - l.f0_max, l.f1_max));
	    continue;

	  case local_vars::ThirdStep:
	    l.step = local_vars::FourthStep;
	    l.g1 = ret;
	    cube.pop();
	    {
	      bdd f0s_min = l.f0_min - l.g0;
	      bdd f1s_min = l.f1_min - l.g1;
	      bdd fs_max = l.f0_max & l.f1_max;
	      bdd fs_min = fs_max & (f0s_min | f1s_min);
	      todo.push(local_vars(fs_min, fs_max));
	    }
	    continue;

	  case local_vars::FourthStep:
	    ret |= (l.g0 - l.v1) | (l.g1 & l.v1);
	    todo.pop();
	    continue;
	  }
	// Unreachable code.
	assert(0);
      }
    return bddfalse;
  }

}
