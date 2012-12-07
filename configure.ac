# $Id: configure.ac,v 1.29 2012/11/25 00:58:19 sarrazip Exp $

AC_PREREQ(2.50)
AC_INIT(src/main.cpp)
AM_INIT_AUTOMAKE(sagasu, 2.0.12)

# Package description:
PACKAGE_FULL_NAME_EN=Sagasu
PACKAGE_FULL_NAME_FR=Sagasu
PACKAGE_SUMMARY_EN="GNOME tool to find strings in a set of files"
PACKAGE_SUMMARY_FR="Outil GNOME cherchant des chaines dans des fichiers"
PACKAGE_CATEGORIES="Utility;"
COPYRIGHT_YEARS="2002-2012"
RELEASE_DATE="2012-11-24"

AC_SUBST(PACKAGE_FULL_NAME_EN)
AC_SUBST(PACKAGE_FULL_NAME_FR)
AC_SUBST(PACKAGE_SUMMARY_EN)
AC_SUBST(PACKAGE_SUMMARY_FR)
AC_SUBST(PACKAGE_CATEGORIES)
AC_SUBST(COPYRIGHT_YEARS)
AC_SUBST(RELEASE_DATE)

PACKAGE_HOME_PAGE="http://sarrazip.com/dev/sagasu.html"
AC_SUBST(PACKAGE_HOME_PAGE)


# Library requirement versions:
LIBGNOMEUI_MINVER=2.8.0; AC_SUBST(LIBGNOMEUI_MINVER)
GTK_MINVER=2.6.0;        AC_SUBST(GTK_MINVER)


AC_PROG_CXX
AC_COMPILE_WARNINGS

AM_GNU_GETTEXT


AC_FUNC_SNPRINTF


PKG_CHECK_MODULES(LIBGNOMEUI, libgnomeui-2.0 >= $LIBGNOMEUI_MINVER)
dnl  PKG_CHECK_MODULES should have declared the following AC_SUBST() clauses,
dnl  but pkg.m4 in version 0.12 of pkg-config does not seem to do it:
AC_SUBST(LIBGNOMEUI_CFLAGS)
AC_SUBST(LIBGNOMEUI_LIBS)

PKG_CHECK_MODULES(GTK, gtk+-2.0 >= $GTK_MINVER)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

AC_PROG_PERL_MODULES(FileHandle DirHandle locale POSIX,
	,
	AC_MSG_ERROR([missing some required Perl modules]))


AC_LANG_CPLUSPLUS

AC_CONFIG_FILES([
	intl/Makefile
	po/Makefile.in
	macros/Makefile
	Makefile
	sagasu.spec
	src/Makefile
	src/sagasu.desktop
])
AC_OUTPUT
