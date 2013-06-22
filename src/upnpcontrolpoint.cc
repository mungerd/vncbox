/* upnpcontrolpoint.cc
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

#include "upnpcontrolpoint.h"
#include "upnp-bindings.h"

void
UPnPControlPoint::service_proxy_available_cb(GUPnPControlPoint *cp,
    GUPnPServiceProxy *proxy, UPnPControlPoint* object)
{
    object->on_service_proxy_available(proxy);
}

UPnPControlPoint::UPnPControlPoint()
{
    m_service_proxy = 0;

    // use current host's default IP
    m_context = gupnp_context_new(NULL, NULL, 0, NULL);

    m_control_point = gupnp_control_point_new(m_context,
        "urn:schemas-upnp-org:service:WANIPConnection:1");

    g_signal_connect(m_control_point, "service-proxy-available",
        G_CALLBACK(service_proxy_available_cb), this);

    // start searching
    gssdp_resource_browser_set_active(GSSDP_RESOURCE_BROWSER(m_control_point), TRUE);
}

UPnPControlPoint::~UPnPControlPoint()
{
    clear_port_mappings();
    g_object_unref(m_control_point);
    g_object_unref(m_context);
}

void
UPnPControlPoint::on_service_proxy_available(GUPnPServiceProxy *proxy)
{
    m_service_proxy = proxy;
    add_queued_port_mappings();
}

void
UPnPControlPoint::clear_port_mappings()
{
    std::list<PortMapping>::iterator it = m_port_mappings.begin();
    while (it != m_port_mappings.end()) {
        delete_upnp_port_mapping(*it);
        it = m_port_mappings.erase(it);
    }
}

void
UPnPControlPoint::add_port_mapping(Protocol protocol,
    unsigned short internal_port,
    unsigned short external_port,
    const std::string& description)
{
    PortMapping pm(protocol, internal_port, external_port, description);
    // call add_upnp_port_mapping(pm) because it could modify pm
    if (m_service_proxy)
        add_upnp_port_mapping(pm);
    m_port_mappings.push_back(pm);
}

void
UPnPControlPoint::delete_port_mapping(Protocol protocol, unsigned short external_port)
{
    std::list<PortMapping>::iterator it = m_port_mappings.begin();
    while (it != m_port_mappings.end()) {
        if (it->protocol == protocol && it->external_port == external_port) {
            delete_upnp_port_mapping(*it);
            it = m_port_mappings.erase(it);
        }
        else
            it++;
    }
}

void
UPnPControlPoint::add_queued_port_mappings()
{
    for (std::list<PortMapping>::iterator it = m_port_mappings.begin();
        it != m_port_mappings.end(); it++)
        add_upnp_port_mapping(*it);
}

void
UPnPControlPoint::add_upnp_port_mapping(PortMapping& pm)
{
    if (!m_service_proxy)
        throw UPnPControlPointError(GUPNP_SERVER_ERROR_OTHER, "service proxy not ready");

    GError *error = NULL;

    while (!upnp_add_port_mapping(
        m_service_proxy,
        NULL,
        pm.external_port,
        get_protocol_string(pm.protocol),
        pm.internal_port,
        gupnp_context_get_host_ip(m_context),
        TRUE,
        pm.description.c_str(),
        0,
        &error)) {

        if (std::string(error->message) == "ConflictInMappingEntry") {
            pm.external_port++;
            g_error_free(error);
            error = NULL;
            continue;
        }

        UPnPControlPointError e(error->code,
            Glib::ustring("AddPortMapping error: ") + error->message);
        g_error_free(error);
        throw e;
    }

    signal_port_mapping_added(get_external_ip(), pm);
}

void
UPnPControlPoint::delete_upnp_port_mapping(const PortMapping& pm)
{
    if (!m_service_proxy)
        throw UPnPControlPointError(GUPNP_SERVER_ERROR_OTHER, "service proxy not ready");

    GError *error = NULL;

    if (!upnp_delete_port_mapping(
        m_service_proxy,
        NULL,
        pm.external_port,
        get_protocol_string(pm.protocol),
        &error)) {

        UPnPControlPointError e(error->code,
            Glib::ustring("DeletePortMapping error: ") + error->message);
        g_error_free(error);
        throw e;
    }
}

const std::string
UPnPControlPoint::get_external_ip() const
{
    if (!m_service_proxy)
        throw UPnPControlPointError(GUPNP_SERVER_ERROR_OTHER, "service proxy not ready");

    std::string ip;

    GError *error = NULL;
    char *ip_ = NULL;

    if (!upnp_get_external_ip_address(m_service_proxy, &ip_, &error)) {
        UPnPControlPointError e(error->code,
            Glib::ustring("GetExternalIPAddress error: ") + error->message);
        g_error_free(error);
        throw e;
    }

    ip = ip_;
    g_free(ip_);

    return ip;
}
