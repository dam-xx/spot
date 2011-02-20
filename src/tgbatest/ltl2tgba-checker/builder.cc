#include "builder.hh"

#include "tgbaalgos/ltl2tgba_fm.hh"
#include "tgbaalgos/ltl2tgba_lacim.hh"
#include "tgbaalgos/ltl2taa.hh"
#include "ltlast/unop.hh"
#include "ltlparse/public.hh"

#include "ltlvisit/tostring.hh"
#include "tgbaalgos/randomgraph.hh"
#include "misc/random.hh"

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

  tgba_map *m = new tgba_map ();
  std::vector<Trad*>::iterator i;
  for (i = algo_.begin (); i != algo_.end (); ++i)
    {
      if ((*i)->do_get ())
	(*m)[(*i)->name_get ()] = tgba_pair ((**i)(f, dict_),
					     (**i)(nf, dict_));
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
