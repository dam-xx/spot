#include "option-handler.hh"

#include <iostream>
#include "misc/random.hh"

#include "trad-taa.hh"
#include "trad-lacim.hh"
#include "trad-fm.hh"

OptionHandler::OptionHandler (int argc, char **argv)
  : desc_ ("Allowed options"),
    trad_ ()
{
  std::string fmd ("use the fm algorithm to compute the automaton\n\
with arg:\n    \
expr :\n    \
symb :\n    \
bran :\n    \
fair :\n    \
none : don't use this algorithm in tests\n");

  std::string taad ("use the taa algorithm to compute the automaton\n\
with arg:\n    \
refi :\n    \
none : don't use this algorithm in tests\n");

  std::string lacimd ("use the lacim algorithm to compute the automaton\n\
with arg:\n    \
none : don't use this algorithm in tests\n");

  std::string eptd ("Choose which emptiness algorithm to use\n\
with arg:\n    \
cou99 : use cou99 algorithm\n    \
tau03 : use Tau03 algorithm\n");

  desc_.add_options()
    ("help,h", "produce help message")
    ("file,f", po::value<std::string> (), "Read ltl formula from file")
    ("intersection", "compute an intersection test")
    ("cross-comparison", "compute a cross comparison test")
    ("consistency", "compute a consistency test")
    ("seed,s", "set seed for the tests")
    ("fm", po::value<std::vector <std::string> >(),fmd.c_str ())
    ("taa", po::value<std::vector <std::string> >(), taad.c_str ())
    ("lacim", po::value<std::vector <std::string> >(), lacimd.c_str ())
    ("emptiness,e", po::value<std::string> (), eptd.c_str ())
    ;

  po::store(po::parse_command_line(argc, argv, desc_), this->vm_);
  po::notify(this->vm_);    

  //Add here you traduction algorithm
  if (vm_.count ("fm"))
    trad_.push_back (new TradFm ("fm",
				 TradFm::optobin (vm_["fm"].as<std::vector<std::string> > ())));
  else
    trad_.push_back (new TradFm("fm", 0));

  if (vm_.count ("taa"))
    trad_.push_back (new TradTaa ("taa",
				  TradTaa::optobin (vm_["taa"].as<std::vector <std::string> >())));
  else
    trad_.push_back (new TradTaa("taa", 0));


  if (vm_.count ("lacim"))
    trad_.push_back (new TradLacim ("lacim",
				    TradLacim::optobin (vm_["lacim"].as<std::vector <std::string> > ())));
  else
    trad_.push_back (new TradLacim("lacim", 0));

}

OptionHandler::~OptionHandler ()
{
  std::cout << "euarg I'm begin destroyed" << std::endl;
}

po::variables_map
OptionHandler::vm_get () const
{
  return this->vm_;
}

po::options_description
OptionHandler::desc_get () const
{
  return this->desc_;
}

std::vector <Trad*>
OptionHandler::trad_get () const
{
  return this->trad_;
}

std::string
OptionHandler::file_get () const
{
  if (vm_.count ("file"))
    return vm_["file"].as<std::string> ();
  else
    return "-";
}

int
OptionHandler::seed_get () const
{
  if (vm_.count ("seed"))
    return vm_["file"].as<int> ();
  else
    return spot::mrand(1231);
}
