AC_PREREQ([2.57])
AC_INIT([spot], [0.1])
AC_CONFIG_AUX_DIR([tools])
AM_INIT_AUTOMAKE([foreign 1.7.3])

AC_PROG_CXX

AC_PROG_RANLIB
AM_PROG_LEX
AC_PROG_YACC

AC_LANG(C++)

CF_GXX_WARNINGS

AC_CHECK_PROG([DOT], [dot], [dot])

AC_CONFIG_FILES([
  Makefile
  doc/Makefile
  doc/Doxyfile
  src/Makefile
  src/ltlenv/Makefile
  src/ltlast/Makefile
  src/ltlparse/Makefile
  src/ltltest/Makefile
  src/ltltest/defs
  src/ltlvisit/Makefile
  src/misc/Makefile
])
AC_OUTPUT