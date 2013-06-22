/* vncboxserver.h
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

#ifndef __vncbox_VNCBOXSERVICE_H
#define __vncbox_VNCBOXSERVICE_H

#include <dbus-c++/dbus.h>
#include <dbus-c++/glib-integration.h>

// FIXME: this is a bug in the dbus-c++ installation
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#include "../src/vncbox-server-glue.h"
#include "notifydata.h"
#include <sigc++/sigc++.h>

class VncBoxService
    : public org::gnome::VncBox_adaptor,
      public DBus::IntrospectableAdaptor,
      public DBus::ObjectAdaptor
{
public:
    static const char* SERVICE_NAME;
    static const char* SERVICE_PATH;

    VncBoxService(DBus::Connection& connection);

    int32_t RequestAccept(const std::map< std::string, std::string >& notifydata);
    void NotifyAfterAccept(const std::map< std::string, std::string >& notifydata);
    void NotifyGone(const std::map< std::string, std::string >& notifydata);

    // signals
    sigc::signal<NotifyResponse, NotifyData> signal_request_accept;
    sigc::signal<void, NotifyData> signal_notify_after_accept;
    sigc::signal<void, NotifyData> signal_notify_gone;
};

#endif // __vncbox_VNCBOXSERVICE_H
