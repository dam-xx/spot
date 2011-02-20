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
 
  const char* error;
  if (opth.vm_get ().count("emptiness") == 0)
    empty_ = spot::emptiness_check_instantiator::construct ("Cou99", &error);
  else if (opth.vm_get ()["emptiness"].as<std::string> () == "cou99")
    empty_ = spot::emptiness_check_instantiator::construct ("Cou99", &error);
  else if (opth.vm_get ()["emptiness"].as <std::string> () == "tau03")
    empty_ = spot::emptiness_check_instantiator::construct ("Tau03_opt", &error);
  else
    std::cerr << "can't recognize emptiness check algorithm : "
	      << opth.vm_get ()["emptiness"].as<std::string> ()
	      << std::endl;
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

spot::emptiness_check_instantiator*
Builder::emptiness_get () const
{
  return (this->empty_);
}

Builder::~Builder ()
{
  //Clean things here if needed
}
