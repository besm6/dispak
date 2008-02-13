#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([besm6], [2.5], [http://groups.google.com/group/besm6/topics])
AM_INIT_AUTOMAKE(besm6, 2.5, no-define)
AC_CONFIG_SRCDIR([extra.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
AC_PROG_CC
AC_PROG_YACC

# Checks for libraries.
AC_CHECK_LIB([m], [sin])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_CHECK_TYPES([ulong])

# Checks for library functions.
AC_CHECK_FUNCS(getopt_long)
AC_OUTPUT
