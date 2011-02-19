#include "builder.hh"

#include "tgbaalgos/ltl2tgba_fm.hh"
#include "tgbaalgos/ltl2tgba_lacim.hh"
#include "tgbaalgos/ltl2taa.hh"
#include "ltlast/unop.hh"
#include "ltlparse/public.hh"

#include "ltlvisit/tostring.hh"
#include "tgbaalgos/randomgraph.hh"
#include "misc/random.hh"
#include <ctime>

Builder::Builder (OptionHandler& opth)
{
  lf_ = new spot::ltl::ltl_file (opth.file_get ());
  dict_ = new spot::bdd_dict ();
  algo_ = opth.trad_get ();
}

BuiltObj*
Builder::operator() ()
{
  spot::ltl::formula* f = lf_->next ();

  if (!f)
    return 0;

  spot::ltl::formula* nf = spot::ltl::unop::instance(spot::ltl::unop::Not,
						     f->clone());

  tgba_map m;
  std::vector<std::string>::iterator i;
  for (i = algo_.begin (); i != algo_.end (); ++i)
    {
      if (*i == "lacim")
	m[*i] = tgba_pair (spot::ltl_to_tgba_lacim (f, dict_),
			   spot::ltl_to_tgba_lacim (nf, dict_));
      else if (*i == "fm")
	m[*i] = tgba_pair (spot::ltl_to_tgba_fm (f, dict_),
			   spot::ltl_to_tgba_fm (nf, dict_));
      else if (*i == "taa")
	m[*i] = tgba_pair (spot::ltl_to_taa (f, dict_),
			   spot::ltl_to_taa (nf, dict_));
    }
  
  spot::ltl::atomic_prop_set* s = spot::ltl::atomic_prop_collect (f, 0);
  spot::tgba* model = spot::random_graph (1, 1, s, dict_, 0, 0.15, 0.5);
  delete s;

  return (new BuiltObj (f, model, m));
}

Builder::~Builder ()
{
  //Clean things here if needed
}

// spot::tgba*
// Builder::build_model (spot::ltl::formula* f)
// {
//   spot::ltl::atomic_prop_set* s = spot::ltl::atomic_prop_collect (f, 0);
//   return spot::random_graph (1,
// 			     1,
// 			     s,
// 			     dict_,
// 			     0,
// 			     0.15,
// 			     0.5);
// }

  // spot::ltl::formula* nf;
  // std::vector<std::string>::iterator i;

  // built b;

  // while ((b->form = lf->next ()))
  // {
  //   nf = spot::ltl::unop::instance(spot::ltl::unop::Not, b->form->clone());
  //   //let's consider we only have one formula for now
  //   for (i = algo.begin (); i != algo.end (); ++i)
  //     {
  // 	if (*i == "lacim")
  // 	  b->m[*i] = tgba_pair (spot::ltl_to_tgba_lacim (b->form, dict_),
  // 			       spot::ltl_to_tgba_lacim (nf, dict_));
  // 	else if (*i == "fm")
  // 	  b->m[*i] = tgba_pair (spot::ltl_to_tgba_fm (b->form, dict_),
  // 			       spot::ltl_to_tgba_fm (nf, dict_));
  // 	else if (*i == "taa")
  // 	  b->m[*i] = tgba_pair (spot::ltl_to_taa (b->form, dict_),
  // 			       spot::ltl_to_taa (nf, dict_));
  //     }
  //   b->model = build_model (b->form);
  //   b_->push_back (b);
  // }
  // spot::srand (opth.seed_get ());
