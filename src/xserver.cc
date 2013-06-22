/* xserver.cc
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

#include "xserver.h"
#include <glibmm.h>
#include <gtkmm.h>
#include <cstdlib>
#include <iostream>

// for kill() and waitpid()
#include <sys/wait.h>
#include <signal.h>

using Glib::ustring;

XServer::XServer(const Gtk::Socket& socket, const Profile& profile)
    : m_socket(socket), m_profile(profile)
{
    m_display = XClientProcess::display = profile.preferred_display;

    m_xserver_process.options = profile.xserver_options;

    for (std::list<ustring>::const_iterator it = profile.anonymous_applications.begin();
        it != profile.anonymous_applications.end(); it++) {
        m_applications[*it] = XClientProcess(get_vncbox_exec() + *it);
        m_applications[*it].signal_exit.connect(bind(sigc::mem_fun(*this,
            &XServer::on_application_exit), *it));
    }
    for (std::map<ustring,ustring>::const_iterator it = profile.self_applications.begin();
        it != profile.self_applications.end(); it++) {
        m_applications[it->first] = XClientProcess(it->second);
        m_applications[it->first].signal_exit.connect(bind(sigc::mem_fun(*this,
            &XServer::on_application_exit), it->first));
    }
    for (std::list<ustring>::const_iterator it = profile.startup_applications.begin();
        it != profile.startup_applications.end(); it++) {
        if (m_applications.find(*it) == m_applications.end()) {
            std::cerr << "skipping startup application " << *it
                << " which has no definition" << std::endl;
        }
        else {
            m_startup_applications.push_back(*it);
        }
    }
}

XServer::~XServer()
{
    stop();
}

void
xserver_process_setup() {
    signal(SIGUSR1, SIG_IGN); // FIXME: member function
}

void
XServer::start()
{
    if (m_xserver_process.running())
        m_xserver_process.terminate();

    // check if is a server is already active on the preferred display
    m_display = m_profile.preferred_display;
    while (!is_display_available(m_display))
        m_display++;

    // prepare Xauthority
    Xau::XauthList auth_list;
    auth_list.load_from_file();
    auth_list.push_back(Xau::Xauth(Xau::LocalAddress(), m_display, m_cookie));
    auth_list.push_back(Xau::Xauth(Xau::InternetAddress(127,0,0,1), m_display, m_cookie));
    auth_list.write_to_file();

    XClientProcess xauth_process(get_vncbox_exec() + "-a " + m_cookie.as_text());
    xauth_process.run();
    xauth_process.wait();

    // prepare process
    m_xserver_process.binary = "Xephyr";
    m_xserver_process.display = m_display;
    m_xserver_process.options = m_profile.xserver_options;
    m_xserver_process.parent_id = m_socket.get_id();

    m_xserver_process.run();
    m_xserver_process.signal_exit.connect(sigc::mem_fun(*this, &XServer::on_xserver_exit));


#if 0
    // wait for server to accept connections
    int itry = 0;
    Glib::RefPtr<Gdk::Display> new_display;
    while ((new_display = Gdk::Display::open(":" + ustring::format(m_display))) == 0) {
        if (itry >= 20) {
            stop();
            throw Glib::SpawnError(Glib::SpawnError::INVAL, "X server not ready"); // FIXME: custom exception
        }
        Glib::usleep(100ul * 1000ul);
        itry++;
    }
    // we won't be using this display
    new_display->close();
#endif

    // launch applications
    XClientProcess::display = m_display;
    for (std::list<ustring>::iterator it = m_startup_applications.begin();
        it != m_startup_applications.end(); it++)
        start_application(*it);

    signal_ready(m_display);
}

void
XServer::stop()
{
    signal_closing(m_display);

    // kill applications
    for (std::map<ustring, XClientProcess>::iterator it = m_applications.begin();
        it != m_applications.end(); it++)
        stop_application(it->first);

    // kill server
    m_xserver_process.terminate();

#if 0
    // detach code
    Glib::RefPtr<Gdk::Window> root = m_socket->get_root_window();
    m_socket->get_plug_window()->reparent(root, 0, 0);
#endif

#if 0
    // wait for X server cleanup
    while (m_xserver_process.running() || !is_display_available(m_display)) {
        std::cout << "waiting for X server to go offline" << std::endl;
        Glib::usleep(100ul * 1000ul);
    }
#endif

    // clean up Xauthority
    Xau::XauthList auth_list;
    auth_list.load_from_file();
    auth_list.remove(Xau::Display(m_display) && m_cookie);
    auth_list.write_to_file();

    XClientProcess xauth_process(get_vncbox_exec() + "-r " + m_cookie.as_text());
    xauth_process.run();
    xauth_process.wait();
}

void
XServer::restart()
{
    stop();
    start();
}

bool
XServer::is_display_available(int display_num)
{
    ustring contents;
    try {
        contents =  Glib::file_get_contents(ustring::compose(
            "/tmp/.X%1-lock", display_num));
    }
    catch (Glib::FileError& e) {
        return true;
    }

    Glib::Pid pid = std::strtoul(contents.c_str(), 0, 0);

    return !Process::running(pid);
}

void
XServer::on_xserver_exit(int status)
{
    std::cout << "server exited; restarting" << std::endl;
    restart();
}

ustring
XServer::get_vncbox_exec() const
{
    return LIBEXECDIR G_DIR_SEPARATOR_S "vncbox-exec -d "
        + ustring::format(m_display) + " ";
}

void
XServer::on_application_exit(int status, const ustring& name)
{
    signal_application_stopped(name);
}

void
XServer::start_application(const ustring& name)
{
    if (m_applications[name].running())
        return;

    m_applications[name].run();
    if (m_applications[name].running())
        signal_application_started(name);
}

void
XServer::stop_application(const ustring& name)
{
    if (!m_applications[name].running())
        return;

    try {
        m_applications[name].terminate();
        signal_application_stopped(name);
    }
    catch (ProcessError& e) {
    }
}
