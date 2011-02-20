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
  desc_.add_options()
    ("help,h", "produce help message")
    ("file,f", po::value<std::string> (), "Read ltl formula from file")
    ("intersection", "compute an intersection test")
    ("cross-comparison", "compute a cross comparison test")
    ("consistency", "compute a consistency check")
    ("seed,s", "set seed for the tests")
    ("fm", "use the fm algorithm to compute the automaton")
    ("taa", "use the taa algorithm to compute the automaton")
    ("lacim", "use the lacim algorithm to compute the automaton")
    ;

  po::store(po::parse_command_line(argc, argv, desc_), this->vm_);
  po::notify(this->vm_);    

  //Add here you traduction algorithm
  trad_.push_back (new TradFm ("fm"));
  trad_.push_back (new TradTaa ("taa"));
  trad_.push_back (new TradLacim ("lacim"));
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
    return spot::mrand(12311231);
}
