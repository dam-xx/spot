#ifndef SPOT_MISC_HASH_HH
#  define SPOT_MISC_HASH_HH

#  include <string>
#  include <functional>

// See the G++ FAQ for details about the following.
#  ifdef __GNUC__
#  if __GNUC__ < 3
#    include <hash_map.h>
#    include <hash_set.h>
    namespace Sgi
    { // inherit globals
      using ::hash_map;
      using ::hash_set;
      using ::hash;
    };
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
  struct ptr_hash :
    public std::unary_function<const T*, size_t>
  {
    size_t operator()(const T* p) const
    {
      return reinterpret_cast<const char*>(p) - static_cast<const char*>(0);
    }
  };

  /// A hash function for strings.
  struct string_hash :
    public Sgi::hash<const char*>,
    public std::unary_function<const std::string&, size_t>
  {
    size_t operator()(const std::string& s) const
    {
      // We are living dangerously.  Be sure to call operator()
      // from the super-class, not this one.
      return Sgi::hash<const char*>::operator()(s.c_str());
    }
  };
}

#endif // SPOT_MISC_HASH_HH
