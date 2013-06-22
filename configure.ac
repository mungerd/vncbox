AC_PREREQ([2.63])
AC_INIT([vncbox], [0.1.0], [mungerd@gmail.com])
AC_CONFIG_SRCDIR([src/vncbox.cc])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AM_MAINTAINER_MODE

AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
PKG_CHECK_MODULES([VNCBOX], [gtkmm-2.4 dbus-c++-1 dbus-c++-glib-1 gupnp-1.0 gssdp-1.0 x11 xau])
PKG_CHECK_MODULES([VNCBOX_EXEC], [glibmm-2.4 xau])
PKG_CHECK_MODULES([VNCBOX_NOTIFY], [dbus-c++-1])

AH_TEMPLATE([HAVE_GCRYPT], [GNU cryptographic library is available])
AM_PATH_LIBGCRYPT([1.4.0],[AC_DEFINE([HAVE_GCRYPT], [1])])

# D-Bus (C++)
AC_PATH_PROG([DBUSXX_XML2CPP],[dbusxx-xml2cpp], [missing])
if test "$DBUSXX_XML2CPP" = "missing"; then
	AC_MSG_ERROR([missing dbusxx-xml2cpp executable])
fi

# GUPnP
AC_PATH_PROG([GUPNP_BINDING_TOOL],[gupnp-binding-tool], [missing])
if test "$GUPNP_BINDING_TOOL" = "missing"; then
	AC_MSG_ERROR([missing gupnp-binding-tool executable])
fi




# Internationalization
IT_PROG_INTLTOOL([0.40.0])
GETTEXT_PACKAGE="$PACKAGE"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"],
                   [The domain to use with gettext])
AM_GLIB_GNU_GETTEXT

# Checks for header files.
AC_CHECK_HEADERS([unistd.h])

AC_CONFIG_FILES([Makefile src/Makefile data/Makefile po/Makefile.in])
AC_OUTPUT
