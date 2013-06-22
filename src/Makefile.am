bin_PROGRAMS = vncbox
libexec_PROGRAMS = vncbox-exec vncbox-notify

xauthxx_srcdir = ${top_srcdir}/xauthxx/src

vncbox_SOURCES = \
    ${xauthxx_srcdir}/xauthxx.h \
    ${xauthxx_srcdir}/xauthxx.cc \
    process.h               process.cc \
    xprocess.h              xprocess.cc \
    serializer.h            serializer.cc \
    randomport.h            randomport.cc \
    profile.h               profile.cc \
    upnp-bindings.h \
    upnpcontrolpoint.h      upnpcontrolpoint.cc \
    xserver.h               xserver.cc \
    vncserver.h             vncserver.cc \
    passwd.h                passwd.cc \
    sysconf.h               sysconf.cc \
    dbus-config.h \
    dbusdaemon.h            dbusdaemon.cc \
    vncbox-server-glue.h \
    vncboxservice.h         vncboxservice.cc \
    displaywatch.h          displaywatch.cc displaywatch-x.cc \
    notifydata.h            notifydata.cc \
    vncserverinfodialog.h   vncserverinfodialog.cc \
    vncclientsinfodialog.h  vncclientsinfodialog.cc \
    vncboxwindow.h          vncboxwindow.cc \
    vncbox.cc

vncbox_exec_SOURCES = \
    ${xauthxx_srcdir}/xauthxx.h \
    ${xauthxx_srcdir}/xauthxx.cc \
    passwd.h        passwd.cc \
    sysconf.h       sysconf.cc \
    process.h       process.cc \
    setuidprocess.h setuidprocess.cc \
    serializer.h    serializer.cc \
    randomport.h    randomport.cc \
    profile.h       profile.cc \
    vncbox-exec.cc

vncbox_notify_SOURCES = \
    notifydata.h    notifydata.cc \
    vncbox-notify.cc

DEFINES = \
    -DLOCALEDIR=\"@localedir@\" \
    -DDATADIR=\"@datadir@/@PACKAGE_NAME@\" \
    -DLIBEXECDIR=\"@libexecdir@\" \
    -DSYSCONFDIR=\"@sysconfdir@\"

AM_CPPFLAGS = ${DEFINES}

vncbox_CPPFLAGS = ${DEFINES} ${VNCBOX_CFLAGS} ${LIBGCRYPT_CFLAGS} -I${xauthxx_srcdir}
vncbox_LDADD = ${VNCBOX_LIBS} ${LIBGCRYPT_LIBS}

vncbox_exec_CPPFLAGS = ${DEFINES} ${VNCBOX_EXEC_CFLAGS} ${LIBGCRYPT_CFLAGS} -I${xauthxx_srcdir}
vncbox_exec_LDADD = ${VNCBOX_EXEC_LIBS} ${LIBGCRYPT_LIBS}

vncbox_notify_CPPFLAGS = ${DEFINES} ${VNCBOX_NOTIFY_CFLAGS} ${LIBGCRYPT_CFLAGS}
vncbox_notify_LDADD = ${VNCBOX_NOTIFY_LIBS} ${LIBGCRYPT_LIBS}

#EXTRA_DIST =

vncbox-server-glue.h: $(top_srcdir)/data/vncbox-introspect.xml Makefile
	$(DBUSXX_XML2CPP) $< --adaptor=$(@F)

vncbox-client-glue.h: $(top_srcdir)/data/vncbox-introspect.xml Makefile
	$(DBUSXX_XML2CPP) $< --proxy=$(@F)

upnp-bindings.h: $(top_srcdir)/data/upnp.xml Makefile
	$(GUPNP_BINDING_TOOL) --prefix=upnp --mode=client $< | \
	sed 's/struct {GCallback cb; gpointer userdata; }/struct upnp_cb_data/' | \
	sed 's/\(G_BEGIN_DECLS\)/\1\n\nstruct upnp_cb_data {GCallback cb; gpointer userdata; };\n/' | \
	sed 's/cbdata =/cbdata = (struct upnp_cb_data *)/' > $(@F)

dbus-config.h: $(top_srcdir)/data/dbus-config.xml Makefile
	( echo "#define VNCBOX_DBUS_CONFIG_STRING ( \\" ; \
	  sed 's/\\/\\\\/g ; s/"/\\"/g ; s/^\(.*\)$$/"\1\\n" \\/' $< ; \
	  echo ")" ) > $(@F)

BUILT_SOURCES = vncbox-server-glue.h vncbox-client-glue.h upnp-bindings.h dbus-config.h
CLEANFILES = $(BUILT_SOURCES)
