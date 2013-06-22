/* vncbox.cc
 *
 * Copyright (C) 2008, 2013  David Munger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 * 	David Munger <mungerd@gmail.com>
 */

#include <glibmm/i18n.h>
#include <gtkmm.h>

#include "vncboxwindow.h"
#include "xserver.h"
#include "vncserver.h"
#include "profile.h"
#include "dbusdaemon.h"
#include "vncboxservice.h"


#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


class VncBoxOptionGroup : public Glib::OptionGroup
{
public:
    VncBoxOptionGroup();
    Glib::OptionGroup::vecustrings remaining;
};


int signal_pipefd[2];

static void
signals_catch(int sig)
{
    // Reference: https://wwwtcs.inf.tu-dresden.de/~tews/Gtk/x2992.html

    if (write(signal_pipefd[1], &sig, sizeof(int)) != sizeof(int))
        std::cerr << "failed to pipe signal " << sig << " to application" << std::endl;
}

static bool
signal_handler(Glib::IOCondition cond)
{
    int sig;
    if (read(signal_pipefd[0], &sig, sizeof(int)) != sizeof(int)) {
        std::cerr << "failed to pipe signal " << sig << " to application" << std::endl;
        return true;
    }

    switch (sig) {
    case SIGINT:
    case SIGHUP:
    case SIGQUIT:
    case SIGTERM:
        std::cout << std::endl << Glib::strsignal(sig) << " -- " << _("TERMINATING")
            << std::endl << std::endl;
        Gtk::Main::quit();
        break;
#ifndef USE_GLIB_CHILD_WATCH
    case SIGCHLD:
	Process::poll_terminated();
        break;
#endif
    }

    return true;
}

static void
signals_setup()
{
    errno = 0;
    if (pipe(signal_pipefd)) {
        std::cerr << "error creating signal pipes: " << Glib::strerror(errno) << std::endl;
        exit(1);
    }
#if 0
    fcntl(signal_pipefd[0], F_SETFL, fcntl(signal_pipefd[0], F_GETFL) | O_NONBLOCK);
    fcntl(signal_pipefd[1], F_SETFL, fcntl(signal_pipefd[1], F_GETFL) | O_NONBLOCK);
    if (errno) {
        std::cerr << "error getting/setting signal pipe flags: "
            << Glib::strerror(errno) << std::endl;
        exit(1);
    }
#endif

    Glib::signal_io().connect(sigc::ptr_fun(&signal_handler), signal_pipefd[0],
        Glib::IO_IN | Glib::IO_HUP);

    signal(SIGINT, signals_catch);
    signal(SIGHUP, signals_catch);
    signal(SIGQUIT, signals_catch);
    signal(SIGTERM, signals_catch);
#ifndef USE_GLIB_CHILD_WATCH
    signal(SIGCHLD, signals_catch);
#endif
}

int
main(int argc, char *argv[])
{
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    Glib::thread_init();

    Glib::OptionContext context("profile_name");
    VncBoxOptionGroup options;
    context.set_main_group(options);

    Gtk::Main kit(argc, argv, context);

    if (options.remaining.size() != 1) {
        std::cerr << context.get_help() << std::flush;
        return -1;
    }
    Glib::ustring profile_name = options.remaining[0];

    signals_setup();

    try {

        Profile profile(profile_name);


        // D-Bus

        DBusDaemon dbus_daemon;
        DBus::Glib::BusDispatcher dbus_dispatcher;
	    DBus::default_dispatcher = &dbus_dispatcher;
        dbus_dispatcher.attach(NULL);
	    DBus::Connection dbus_connection = DBus::Connection(dbus_daemon.get_address().c_str());
	    dbus_connection.register_bus();
	    dbus_connection.request_name(VncBoxService::SERVICE_NAME);


         // main window & X server

        VncBoxWindow mainwindow;
        XServer xserver(mainwindow.get_socket(), profile);

        mainwindow.set_applications_list(profile.applications_titles);
        mainwindow.signal_socket_ready.connect(sigc::mem_fun(xserver, &XServer::start));
        mainwindow.signal_xserver_restart.connect(sigc::mem_fun(xserver, &XServer::restart));

        mainwindow.get_socket().set_size_request(profile.window_width, profile.window_height);

        xserver.signal_ready.connect(sigc::mem_fun(mainwindow, &VncBoxWindow::on_xserver_ready));
        xserver.signal_closing.connect(sigc::mem_fun(mainwindow, &VncBoxWindow::on_xserver_closing));


        // applications signals

        xserver.signal_application_started.connect(sigc::bind(sigc::mem_fun(mainwindow,
            &VncBoxWindow::set_application_state), true));
        xserver.signal_application_stopped.connect(sigc::bind(sigc::mem_fun(mainwindow,
            &VncBoxWindow::set_application_state), false));

        mainwindow.signal_application_start.connect(sigc::mem_fun(xserver,
            &XServer::start_application));
        mainwindow.signal_application_stop.connect(sigc::mem_fun(xserver,
            &XServer::stop_application));


        // D-Bus service

        VncBoxService service(dbus_connection);
        service.signal_request_accept.connect(sigc::mem_fun(mainwindow,
            &VncBoxWindow::on_request_accept));
        service.signal_notify_after_accept.connect(sigc::mem_fun(mainwindow,
            &VncBoxWindow::on_notify_after_accept));
        service.signal_notify_gone.connect(sigc::mem_fun(mainwindow,
            &VncBoxWindow::on_notify_gone));


        // UPnP control point

        UPnPControlPoint upnp_control_point;


        // VNC server

        VncServer vnc_server(profile, upnp_control_point);
        vnc_server.set_dbus_address(dbus_daemon.get_address());

        vnc_server.signal_ready.connect(sigc::mem_fun(mainwindow,
            &VncBoxWindow::on_vnc_server_ready));
        vnc_server.signal_closing.connect(sigc::bind(sigc::mem_fun(mainwindow,
            &VncBoxWindow::set_vnc_state), false));
        vnc_server.signal_upnp_timeout.connect(sigc::mem_fun(mainwindow,
            &VncBoxWindow::confirm_no_upnp));

        mainwindow.signal_vnc_start.connect(sigc::mem_fun(vnc_server, &VncServer::start));
        mainwindow.signal_vnc_stop.connect(sigc::mem_fun(vnc_server, &VncServer::stop));

        xserver.signal_ready.connect(sigc::mem_fun(vnc_server, &VncServer::set_display));
        xserver.signal_ready.connect(sigc::hide(sigc::mem_fun(vnc_server, &VncServer::start)));
        xserver.signal_closing.connect(sigc::hide(sigc::mem_fun(vnc_server, &VncServer::stop)));


        // run application

        Gtk::Main::run(mainwindow);

    }
    catch (const Glib::Exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}

VncBoxOptionGroup::VncBoxOptionGroup()
    : Glib::OptionGroup("vncbox", "VNC Box", "VNC Box options")
{
    Glib::OptionEntry entry_remaining;
    entry_remaining.set_long_name(G_OPTION_REMAINING);
    entry_remaining.set_arg_description(G_OPTION_REMAINING);

    add_entry(entry_remaining, remaining);
}
