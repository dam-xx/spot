#ifndef SPOT_MISC_CONSTSEL_HH
# define SPOT_MISC_CONSTSEL_HH

namespace spot 
{
  // Be default, define the type as-is.
  template <class T, bool WantConst>
  struct const_sel
  {
    typedef T t;
  };
  
  // If const is wanted, add it.
  template <class T>
  struct const_sel<T, true>
  {
    typedef const T t;
  };
  
  // If const is present but isn't wanted, remove it.
  template <class T>
  struct const_sel<const T, false>
  {
    typedef const T t;
  };
}

#endif // SPOT_MISC_CONSTSEL_HH
