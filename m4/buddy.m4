AC_DEFUN([AX_CHECK_BUDDY], [
  AC_ARG_WITH([included-buddy],
	      [AC_HELP_STRING([--with-included-buddy],
			      [use the BuDDy library inclued here])])
  AC_CHECK_LIB([bdd], [bdd_mergepairs],
	       [need_included_buddy=no],
	       [need_included_buddy=yes])

  if test "$need_included_buddy" = yes; then
     if test "$with_included_buddy" = no; then
       AC_MSG_ERROR([Could not link with BuDDy.  Please install BuDDy first,
		    set CPPFLAGS/LDFLAGS appropriately, or configure with
		    --with-included-buddy])
     else
	with_included_buddy=yes
     fi
  fi

  if test "$with_included_buddy" = yes;  then
     AC_CONFIG_SUBDIRS([buddy])
     BUDDY_LDFLAGS='$(top_builddir)/buddy/src/libbdd.la'
     BUDDY_CPPFLAGS='-I$(top_srcdir)/buddy/src'
  else
     BUDDY_LDFLAGS='-lbdd'
  fi
  AM_CONDITIONAL([WITH_INCLUDED_BUDDY], [test "$with_included_buddy" = yes])
  AC_SUBST([BUDDY_LDFLAGS])
  AC_SUBST([BUDDY_CPPFLAGS])
])
