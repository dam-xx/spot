#ifndef SPOT_MISC_HASH_HH
#  define SPOT_MISC_HASH_HH

// See the G++ FAQ for details about this.

#  ifdef __GNUC__
#  if __GNUC__ < 3
#    include <hash_map.h>
#    include <hash_set.h>
    namespace Sgi { using ::hash_map; }; // inherit globals
#  else
#    include <ext/hash_map>
#    include <ext/hash_set>
#    if __GNUC_MINOR__ == 0
      namespace Sgi = std;               // GCC 3.0
#    else
      namespace Sgi = ::__gnu_cxx;       // GCC 3.1 and later
#    endif
#  endif
#  else      // ...  there are other compilers, right?
#   include <hash_map>
#   include <hash_set>
    namespace Sgi = std;
#  endif

namespace spot
{

  /// A hash function for pointers.
  template <class T>
  struct ptr_hash
  {
    size_t operator()(const T* f) const
    {
      return reinterpret_cast<const char*>(f) - static_cast<const char*>(0);
    }
  };
}

#endif // SPOT_MISC_HASH_HH
