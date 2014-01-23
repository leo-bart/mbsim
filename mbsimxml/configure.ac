dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.57])
AC_INIT([mbsimxml],[2.0.1],[friedrich.at.gc@googlemail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([mbsimxml/mbsimflatxml.cc])
LT_INIT([win32-dll])
AC_PROG_CXX
AC_PROG_CXXCPP
AC_LANG([C++])

PKG_CHECK_MODULES(DEPS, mbsim openmbvcppinterface)
PKG_CHECK_MODULES(NOLINK_DEPS, mbxmlutils)

if test "_$host_os" = "_mingw32" ; then
  AC_DEFINE([MBSIMXML_MINGW], [1], [Host System is MinGW/MSys])
  MBSIMXML_MINGW=""
else
  MBSIMXML_MINGW="-fPIC"
fi
AC_SUBST([MBSIMFLATXML_CFLAGS])

dnl boost
AC_CHECK_HEADERS([boost/filesystem.hpp], [], [AC_MSG_ERROR(boost/filesystem.hpp not found.)])

AC_ARG_WITH([boost-filesystem-lib],
  [  --with-boost-filesystem-lib=LIBNAME  The library name of boost filesystem excluding prefix/suffix (Default: boost_filesystem).],
  [BOOST_FILESYSTEM_LIB="$withval"], [BOOST_FILESYSTEM_LIB="boost_filesystem"])
AC_SUBST([BOOST_FILESYSTEM_LIB])
AC_ARG_WITH([boost-system-lib],
  [  --with-boost-system-lib=LIBNAME  The library name of boost system excluding prefix/suffix (Default: boost_system).],
  [BOOST_SYSTEM_LIB="$withval"], [BOOST_SYSTEM_LIB="boost_system"])
AC_SUBST([BOOST_SYSTEM_LIB])

AC_MSG_CHECKING([for WEXITSTATUS])
AC_COMPILE_IFELSE([
#include <stdlib.h>
int main() {
  int j=1;
  int i=WEXITSTATUS(j);
  return 0;
}
], [AC_MSG_RESULT([yes]); CPPFLAGS="$CPPFLAGS -DHAVE_WEXITSTATUS"], [AC_MSG_RESULT([no])])

dnl Check for xslt
AC_ARG_WITH([xsltprocpath],
  [  --with-xsltproc-path=PATHTOXSLTPROC  The path to the 'xsltproc' program.],
  [xsltprocpath="$withval"],[xsltprocpath=""])
AC_PATH_PROG([xsltproc],[xsltproc],[no],[$xsltprocpath:$prefix/bin:$PATH])
test $xsltproc = "no" && AC_MSG_ERROR([xsltproc not found])
AC_SUBST([XSLTPROC],[$xsltproc])

dnl Check for xmllint
AC_ARG_WITH([xmllintpath],
  [  --with-xmllint-path=PATHTOXMLLINT  The path to the 'xmllint' program.],
  [xmllintpath="$withval"],[xmllintpath=""])
AC_PATH_PROG([xmllint],[xmllint],[no],[$xmllintpath:$prefix/bin:$PATH])
test $xmllint = "no" && AC_MSG_ERROR([xmllint not found])
AC_SUBST([XMLLINT],[$xmllint])



dnl XML
AC_SUBST([MBXMLUTILSBINDIR],[$(pkg-config mbxmlutils --variable BINDIR)])
AC_SUBST([MBXMLUTILSSCHEMA],[$(pkg-config mbxmlutils --variable SCHEMADIR)])



dnl MBXMLUtilsDOCDIR
MBXMLUTILSDOCDIR=$(pkg-config mbxmlutils --variable DOCDIR)
AC_SUBST([MBXMLUTILSDOCDIR],[$MBXMLUTILSDOCDIR])



CPPFLAGS="$CPPFLAGS -pedantic -Wall -Werror -Wno-unknown-pragmas -Wno-long-long"

LDFLAGS="$LDFLAGS -no-undefined -Wl,--no-undefined"

AC_CONFIG_FILES([Makefile mbsimxml/Makefile xmldoc/Makefile mbsimxml.pc mbsimxml/env.h])

AC_OUTPUT

echo "
Configuration for $PACKAGE $VERSION :
--------------------------------

 Source code location ....: $srcdir
 Host System Type ........: $host
 Preprocessor ............: $CC $CPPFLAGS
 C Compiler ..............: $CC $CFLAGS $LIBFLAC_CFLAGS
 C++ Compiler ............: $CXX $CXXFLAGS
 Fortran 77 Compiler .....: $F77 $FFLAGS
 Fortran 90 Compiler .....: $FC $FCFLAGS
 Linker ..................: $CC $LDFLAGS $LIBS $LIBFLAC_LIBS
 Install path ............: $prefix

 Now type 'make' to build $PACKAGE $VERSION,
 and then 'make install' for installation.
"