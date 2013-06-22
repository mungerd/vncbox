/* xserver.h
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

#ifndef __vncbox_XSERVER_H
#define __vncbox_XSERVER_H

#include <glibmm.h>
#include <gtkmm/socket.h>
#include "profile.h" // FIXME
#include "xprocess.h"
#include "xauthxx.h"

class XServer
{

public:
    XServer(const Gtk::Socket& socket, const Profile& profile); // FIXME: set_preferred_display() and add(client) instead profile ref
    virtual ~XServer();

    void start();
    void stop();
    void restart();

    static bool is_display_available(int display_num);
    sigc::signal<void,unsigned short> signal_ready;
    sigc::signal<void,unsigned short> signal_closing;

    void start_application(const Glib::ustring& name);
    void stop_application(const Glib::ustring& name);
    sigc::signal<void,Glib::ustring> signal_application_started;
    sigc::signal<void,Glib::ustring> signal_application_stopped;

    unsigned short get_display() const
        { return m_display; }

protected:
    const Gtk::Socket& m_socket;
    const Profile& m_profile;
    unsigned short m_display;
    XServerProcess m_xserver_process;
    std::map<Glib::ustring, XClientProcess> m_applications;
    std::list<Glib::ustring> m_startup_applications;
    Xau::MagicCookie m_cookie;

    void on_xserver_exit(int status);
    void on_application_exit(int status, const Glib::ustring& name);

    Glib::ustring get_vncbox_exec() const;
};

#endif // __vncbox_XSERVER_H
