/* upnpcontrolpoint.h
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

#ifndef __vncbox_UPNPCONTROLPOINT_H
#define __vncbox_UPNPCONTROLPOINT_H

#include <libgupnp/gupnp.h>
#include <sigc++/sigc++.h>
#include <glibmm.h>
#include <list>

class UPnPControlPointError : public Glib::Exception
{
public:
    enum Code {
        CANNOT_READ_FILE
    };

    const int code;
    const Glib::ustring message;

    UPnPControlPointError(int error_code, const Glib::ustring& error_message)
        : code(error_code), message(error_message) {}

    virtual ~UPnPControlPointError() throw() {}

    virtual Glib::ustring what() const { return message; }
};

class UPnPControlPoint
{
public:
    enum Protocol {
        PROTOCOL_TCP,
        PROTOCOL_UDP
    };

    struct PortMapping
    {
        Protocol protocol;
        unsigned short internal_port;
        unsigned short external_port;
        std::string description;

        PortMapping(Protocol protocol = PROTOCOL_TCP,
            unsigned short internal_port = 0,
            unsigned short external_port = 0,
            const std::string& description = std::string())
            { this->protocol = protocol;
              this->internal_port = internal_port;
              this->external_port = external_port;
              this->description = description; }
    };


    UPnPControlPoint();
    ~UPnPControlPoint();

    void add_port_mapping(Protocol protocol, unsigned short internal_port,
        unsigned short external_port, const std::string& description);

    void delete_port_mapping(Protocol protocol, unsigned short external_port);

    void clear_port_mappings();

    const std::string get_external_ip() const;

    sigc::signal<void, std::string, PortMapping> signal_port_mapping_added; // (external IP, pm)

protected:

    GUPnPContext* m_context;
    GUPnPControlPoint* m_control_point;
    GUPnPServiceProxy *m_service_proxy;

    std::list<PortMapping> m_port_mappings;
    std::string m_external_ip;

    void on_service_proxy_available(GUPnPServiceProxy *proxy);
    void add_upnp_port_mapping(PortMapping& pm);
    void delete_upnp_port_mapping(const PortMapping& pm);
    void add_queued_port_mappings();

    static void service_proxy_available_cb(GUPnPControlPoint *cp,
        GUPnPServiceProxy *proxy, UPnPControlPoint* object);

    static const char* get_protocol_string(Protocol protocol)
        { return protocol == PROTOCOL_UDP ? "UDP" : "TCP"; }
};

#endif // __vncbox_UPNPCONTROLPOINT_H
