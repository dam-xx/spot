#ifndef OPTION_HANDLER_HH_
# define OPTION_HANDLER_HH_

#include <vector>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include "trad.hh"

namespace po = boost::program_options;

class OptionHandler
{
public:
  OptionHandler (int argc, char** argv);
  ~OptionHandler ();

public:
  po::variables_map vm_get () const;
  po::options_description desc_get () const;
  std::vector <Trad*> trad_get () const;
  std::string file_get () const;
  int seed_get () const;

protected:
  po::options_description desc_;
  po::variables_map vm_;
  std::vector<Trad*> trad_;


};

#endif /* !OPTION_HANDLER_HH_ */
