AC_DEFUN([AX_CHECK_BUDDY], [
  AC_CHECK_LIB([bdd], [bdd_init],, 
    [AC_MSG_ERROR([Could not link with BuDDy.  Please install BuDDy first 
                   or set CPPFLAGS/LDFLAGS appropriately.])])])
