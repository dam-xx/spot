// Copyright (C) 2009 Laboratoire d'Informatique de Paris
// 6 (LIP6), département Systèmes Répartis Coopératifs (SRC),
// Université Pierre et Marie Curie.
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

#include <utility>
#include "ltlast/visitor.hh"
#include "ltlast/allnodes.hh"
#include "ltlvisit/lunabbrev.hh"
#include "ltlvisit/tunabbrev.hh"
#include "ltlvisit/nenoform.hh"
#include "ltlvisit/destroy.hh"
#include "ltlvisit/tostring.hh"
#include "ltlvisit/clone.hh"
#include "ltl2taa.hh"

namespace spot
{
  namespace
  {
    using namespace ltl;

    /// \brief Recursively translate a formula into a TAA.
    class ltl2taa_visitor : public const_visitor
    {
    public:
      ltl2taa_visitor(taa* res, bool negated = false)
	: res_(res), negated_(negated), init_(), succ_(), to_free_()
      {
      }

      virtual
      ~ltl2taa_visitor()
      {
      }

      taa*
      result()
      {
	for (unsigned i = 0; i < to_free_.size(); ++i)
	  destroy(to_free_[i]);
	res_->set_init_state(init_);
	return res_;
      }

      void
      visit(const atomic_prop* node)
      {
	const formula* f = node; // Handle negation
	if (negated_)
	{
	  f = unop::instance(unop::Not, clone(node));
	  to_free_.push_back(f);
	}
	init_ = to_string(f);
	std::vector<std::string> dst;

	dst.push_back(std::string("well"));
	taa::transition* t = res_->create_transition(init_, dst);
	res_->add_condition(t, clone(f));

	succ_.push_back(std::make_pair(dst, f));
      }

      void
      visit(const constant* node)
      {
	init_ = to_string(node);
	std::vector<std::string> dst;
	switch (node->val())
	{
	  case constant::True:
	    dst.push_back(std::string("well"));
	    res_->create_transition(init_, dst);
	    succ_.push_back(std::make_pair(dst, node));
	    return;
	  case constant::False:
	    return;
	}
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(const unop* node)
      {
	negated_ = node->op() == unop::Not;
	ltl2taa_visitor v = recurse(node->child());

	init_ = to_string(node);
	std::vector<std::string> dst;
	switch (node->op())
	{
	  case unop::X:
	    if (v.succ_.empty()) // Handle X(0)
	      return;
	    dst.push_back(v.init_);
	    res_->create_transition(init_, dst);
	    succ_.push_back(std::make_pair(dst, constant::true_instance()));
	    return;
	  case unop::F:
	  case unop::G:
	    assert(0); // TBD
	    return;
	  case unop::Not:
	    // Done in recurse
	    succ_ = v.succ_;
	    return;
	  case unop::Finish:
	    assert(!"unsupported operator");
	}
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(const binop* node)
      {
	ltl2taa_visitor v1 = recurse(node->first());
	ltl2taa_visitor v2 = recurse(node->second());

	init_ = to_string(node);
	std::vector<succ_state>::iterator i1;
	std::vector<succ_state>::iterator i2;
	taa::transition* t = 0;
	switch (node->op())
	{
	  case binop::U: // Strong
	    for (i1 = v1.succ_.begin(); i1 != v1.succ_.end(); ++i1)
	    {
	      i1->first.push_back(init_); // Add the initial state
	      t = res_->create_transition(init_, i1->first);
	      res_->add_condition(t, clone(i1->second));
	      res_->add_acceptance_condition(t, clone(node));
	      succ_.push_back(*i1);
	    }
	    for (i2 = v2.succ_.begin(); i2 != v2.succ_.end(); ++i2)
	    {
	      t = res_->create_transition(init_, i2->first);
	      res_->add_condition(t, clone(i2->second));
	      succ_.push_back(*i2);
	    }
	    return;
	  case binop::R: // Weak
	    for (i2 = v2.succ_.begin(); i2 != v2.succ_.end(); ++i2)
	    {
	      for (i1 = v1.succ_.begin(); i1 != v1.succ_.end(); ++i1)
	      {
		std::vector<std::string> u; // Union
		std::copy(i1->first.begin(), i1->first.end(), ii(u, u.begin()));
		std::copy(i2->first.begin(), i2->first.end(), ii(u, u.begin()));
		const formula* f = multop::instance
		  (multop::And, clone(i1->second), clone(i2->second));
		to_free_.push_back(f);

		t = res_->create_transition(init_, u);
		res_->add_condition(t, clone(f));
		succ_.push_back(std::make_pair(u, f));
	      }

	      i2->first.push_back(init_); // Add the initial state
	      t = res_->create_transition(init_, i2->first);
	      res_->add_condition(t, clone(i2->second));
	      succ_.push_back(*i2);
	    }
	    return;
	  case binop::Xor:
	  case binop::Implies:
	  case binop::Equiv:
	    assert(0); // TBD
	}
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(const multop* node)
      {
	bool ok = true;
	std::vector<ltl2taa_visitor> vs;
	for (unsigned n = 0; n < node->size(); ++n)
	{
	  vs.push_back(recurse(node->nth(n)));
	  if (vs[n].succ_.empty()) // Handle 0
	    ok = false;
	}

	init_ = to_string(node);
	std::vector<succ_state>::iterator i;
	taa::transition* t = 0;
	switch (node->op())
	{
	  case multop::And:
	  {
	    std::vector<succ_state> p = all_n_tuples(vs);
	    for (unsigned n = 0; n < p.size() && ok; ++n)
	    {
	      t = res_->create_transition(init_, p[n].first);
	      res_->add_condition(t, clone(p[n].second));
	      succ_.push_back(p[n]);
	    }
	    return;
	  }
	  case multop::Or:
	    for (unsigned n = 0; n < node->size(); ++n)
	      for (i = vs[n].succ_.begin(); i != vs[n].succ_.end(); ++i)
	      {
		t = res_->create_transition(init_, i->first);
		res_->add_condition(t, clone(i->second));
		succ_.push_back(*i);
	      }
	    return;
	}
	/* Unreachable code.  */
	assert(0);
      }

      void
      visit(const automatop* node)
      {
	(void) node;
	assert(!"unsupported operator");
      }

      ltl2taa_visitor
      recurse(const formula* f)
      {
	ltl2taa_visitor v(res_, negated_);
	f->accept(v);
	for (unsigned i = 0; i < v.to_free_.size(); ++i)
	  to_free_.push_back(v.to_free_[i]);
	return v;
      }

    private:
      taa* res_;
      bool negated_;

      typedef std::insert_iterator<
	std::vector<std::string>
      > ii;

      typedef std::pair<
        std::vector<std::string>, const formula*
      > succ_state;

      std::string init_;
      std::vector<succ_state> succ_;

      std::vector<const formula*> to_free_;

    public:
      std::vector<succ_state>
      all_n_tuples(const std::vector<ltl2taa_visitor>& vs)
      {
	std::vector<succ_state> product;

	std::vector<int> pos;
	for (unsigned i = 0; i < vs.size(); ++i)
	  pos.push_back(vs[i].succ_.size());

	while (pos[0] != 0)
	{
	  std::vector<std::string> u; // Union
	  formula* f = constant::true_instance();
	  for (unsigned i = 0; i < vs.size(); ++i)
	  {
	    if (vs[i].succ_.empty())
	      continue;
	    const succ_state& ss(vs[i].succ_[pos[i] - 1]);
	    std::copy(ss.first.begin(), ss.first.end(), ii(u, u.begin()));
	    f = multop::instance(multop::And, clone(ss.second), f);
	  }
	  to_free_.push_back(f);
	  product.push_back(std::make_pair(u, f));

	  for (int i = vs.size() - 1; i >= 0; --i)
	  {
	    if (vs[i].succ_.empty())
	      continue;
	    if (pos[i] > 1 || (i == 0 && pos[0] == 1))
	    {
	      --pos[i];
	      break;
	    }
	    else
	      pos[i] = vs[i].succ_.size();
	  }
	}
	return product;
      }
    };
  } // anonymous

  taa*
  ltl_to_taa(const ltl::formula* f, bdd_dict* dict)
  {
    // TODO: s/unabbreviate_ltl/unabbreviate_logic/
    const ltl::formula* f1 = ltl::unabbreviate_ltl(f);
    const ltl::formula* f2 = ltl::negative_normal_form(f1);
    ltl::destroy(f1);

    std::cerr << ltl::to_string(f2) << std::endl;

    taa* res = new spot::taa(dict);
    ltl2taa_visitor v(res);
    f2->accept(v);
    ltl::destroy(f2);

    return v.result();
  }
}
