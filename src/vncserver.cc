/* vncserver.cc
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

#include "vncserver.h"

#include <iostream>

using Glib::ustring;

VncServer::VncServer(const Profile& profile, UPnPControlPoint& upnp)
    : m_profile(profile), m_upnp(upnp)
{
    m_external_rfb_port = m_external_http_port = 0;
    m_display = -1;
    m_server_key = "x"; // FIXME
}

VncServer::~VncServer()
{
    stop();
}

void
VncServer::set_display(int display)
{
    if (m_vnc_server_process.running() && m_display != display) {
        stop();
        m_display = display;
        start();
    }
    else
        m_display = display;
}

void
VncServer::set_dbus_address(ustring dbus_address)
{
    if (m_vnc_server_process.running() && m_dbus_address != dbus_address) {
        stop();
        m_dbus_address = dbus_address;
        start();
    }
    else
        m_dbus_address = dbus_address;
}

void
VncServer::start()
{
    if (m_vnc_server_process.running())
        return;

    if (m_display < 0) {
        std::cerr << "warning: trying to start VNC server with uninitialized display number"
            << std::endl;
        return;
    }
    if (m_dbus_address.empty()) {
        std::cerr << "warning: trying to start VNC server with uninitialized D-Bus address"
            << std::endl;
        return;
    }
    if (m_server_key.empty()) {
        std::cerr << "warning: trying to start VNC server with uninitialized server key"
            << std::endl;
        return;
    }

    m_vnc_server_process.command_line = get_vncbox_exec()
        + "-b " + m_dbus_address + " -k " + m_server_key + " vnc-server";

    m_vnc_server_process.run(false, Process::PIPE_INPUT);
    m_vnc_server_process.signal_exit.connect(sigc::mem_fun(*this, &VncServer::on_vnc_server_exit));

    try {
        m_profile.write_to(m_vnc_server_process.get_input_channel());
        m_vnc_server_process.get_input_channel()->close();
    }
    catch (Glib::Exception& e) {
        std::cerr << "Error sending profile information to VNC server process: "
            << e.what() << std::endl;
    }

    m_external_ip = std::string();
    m_external_rfb_port = m_external_http_port = 0;

    if (m_profile.upnp)
	start_with_upnp();
    else
	start_without_upnp();
}

void
VncServer::start_with_upnp()
{
    m_upnp.signal_port_mapping_added.connect(sigc::mem_fun(*this,
		&VncServer::on_upnp_port_mapping_added));

    m_upnp.add_port_mapping(UPnPControlPoint::PROTOCOL_TCP, m_profile.rfb_port,
	    m_profile.rfb_port, std::string("VNC Box (") + g_get_host_name()
	    + ":" + ustring::format(m_display) + ") [RFB]");

    // HTTP SSL uses the same port as RFB
    if (m_profile.http && !m_profile.http_ssl)
	m_upnp.add_port_mapping(UPnPControlPoint::PROTOCOL_TCP, m_profile.http_port,
		m_profile.http_port, std::string("VNC Box (") + g_get_host_name()
		+ ":" + ustring::format(m_display) + ") [HTTP]");

    Glib::signal_timeout().connect(sigc::mem_fun(*this, &VncServer::on_upnp_timeout), 3500);
}

void
VncServer::start_without_upnp()
{
    signal_ready(
	    g_get_host_name(),
	    m_profile.rfb_port,
	    m_profile.http_ssl ? m_profile.rfb_port : m_profile.http_port,
	    m_profile.http_ssl,
	    m_profile.password);
}

void
VncServer::stop()
{
    if (!m_vnc_server_process.running())
        return;

    signal_closing();
    m_vnc_server_process.terminate();
}

ustring
VncServer::get_vncbox_exec() const
{
    return LIBEXECDIR G_DIR_SEPARATOR_S "vncbox-exec -d "
        + ustring::format(m_display) + " ";
}

bool
VncServer::on_upnp_timeout()
{
    if (signal_upnp_timeout()) {
	std::cerr << "WARNING: cannot find UPnP service; ignoring UPnP setting" << std::endl;
	start_without_upnp();
	return false;
    }
    else {
	return true;
    }
}

void
VncServer::on_vnc_server_exit(int status)
{
    signal_closing();
}

void
VncServer::on_upnp_port_mapping_added(const std::string& external_ip,
    const UPnPControlPoint::PortMapping& pm)
{
    m_external_ip = external_ip;

    if (pm.description.find("[RFB]") != std::string::npos)
        m_external_rfb_port = pm.external_port;
    else if (pm.description.find("[HTTP]") != std::string::npos)
        m_external_http_port = pm.external_port;

    if (m_external_rfb_port && (!m_profile.http || m_profile.http_ssl || m_external_http_port))
        signal_ready(
            external_ip,
            m_external_rfb_port,
            m_profile.http_ssl ? m_external_rfb_port : m_external_http_port,
            m_profile.http_ssl,
            m_profile.password);
}
