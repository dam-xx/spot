AC_PREREQ([2.57])
AC_INIT([spot], [0.0k])
AC_CONFIG_AUX_DIR([tools])
AM_INIT_AUTOMAKE([foreign nostdinc check-news 1.7.3])

adl_ENABLE_DEVEL
adl_CHECK_PYTHON

AC_PROG_CXX

AM_PROG_LEX
AC_PROG_YACC

AC_LANG(C++)

AX_CHECK_BUDDY
AX_CHECK_LBTT
AX_CHECK_GSPNLIB

AC_PROG_LIBTOOL

CF_GXX_WARNINGS
adl_ENABLE_DEBUG
ad_GCC_OPTIM
adl_NDEBUG

AC_CHECK_PROG([DOT], [dot], [dot])

AC_CONFIG_FILES([
  Makefile
  doc/Makefile
  doc/Doxyfile
  iface/Makefile
  iface/gspn/Makefile
  iface/gspn/defs
  src/Makefile
  src/ltlenv/Makefile
  src/ltlast/Makefile
  src/ltlparse/Makefile
  src/ltltest/Makefile
  src/ltltest/defs
  src/ltlvisit/Makefile
  src/tgba/Makefile
  src/tgbaalgos/Makefile
  src/tgbaparse/Makefile
  src/tgbatest/Makefile
  src/tgbatest/defs
  src/misc/Makefile
  wrap/Makefile
  wrap/python/Makefile
  wrap/python/cgi/Makefile
  wrap/python/tests/Makefile
])
AC_CONFIG_FILES([wrap/python/tests/run], [chmod +x wrap/python/tests/run])
AC_OUTPUT
