/* vncserver.h
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

#ifndef __vncbox_VNCSERVER_H
#define __vncbox_VNCSERVER_H

#include "process.h"
#include "profile.h"
#include "upnpcontrolpoint.h"

class VncServer
{
public:
    VncServer(const Profile& profile, UPnPControlPoint& upnp);
    ~VncServer();
    void start();
    void stop();

    void set_display(int display);
    void set_dbus_address(Glib::ustring dbus_address);

    int get_display() const
        { return m_display; }

    sigc::signal<void, std::string, unsigned short, unsigned short, bool, std::string> signal_ready;
    sigc::signal<void> signal_closing;
    sigc::signal<bool> signal_upnp_timeout;


protected:
    int m_display;
    const Profile& m_profile;
    UPnPControlPoint& m_upnp;

    CommandLineProcess m_vnc_server_process;
    Glib::ustring m_dbus_address;
    Glib::ustring m_server_key;

    std::string m_external_ip;
    unsigned short m_external_rfb_port;
    unsigned short m_external_http_port;

    Glib::ustring get_vncbox_exec() const;
    void on_vnc_server_exit(int status);
    void on_upnp_port_mapping_added(const std::string& external_ip,
        const UPnPControlPoint::PortMapping& pm);

    void start_with_upnp();
    void start_without_upnp();
    bool on_upnp_timeout();
};

#endif // __vncbox_VNCSERVER_H
