#ifndef SPOT_LTLVISIT_TOSTRING_HH
# define SPOT_LTLVISIT_TOSTRING_HH

#include <ltlast/formula.hh>
#include <iostream>

namespace spot
{
  namespace ltl
  {
    /// \brief Output a formula as a (parsable) string.
    /// \param f The formula to translate.
    /// \param os The stream where it should be output.
    std::ostream& to_string(const formula* f, std::ostream& os);

    /// \brief Convert a formula into a (parsable) string.
    /// \param f The formula to translate.
    std::string to_string(const formula* f);
  }
}

#endif // SPOT_LTLVISIT_TOSTRING_HH
