# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)

AC_INIT(XFITSview, 2.0, bcotton@nrao.edu)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR(src/imagedisp.c)

# Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.
AC_PATH_XTRA
AC_CHECK_LIB(m, pow)
AC_PATH_MOTIF

# Checks for system services.
AC_SYS_LARGEFILE

AC_CONFIG_FILES([Makefile src/Makefile fitssubs/Makefile])
AC_OUTPUT
