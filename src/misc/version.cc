#include "version.hh"

namespace spot
{
  static const char version_[] = VERSION;

  const char* 
  version()
  {
    return version_;
  }
}
