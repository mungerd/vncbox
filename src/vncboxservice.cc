/* vncboxserver.cc
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


#include "vncboxservice.h"
#include "notifydata.h"

const char* VncBoxService::SERVICE_NAME = "org.gnome.VncBox";
const char* VncBoxService::SERVICE_PATH = "/org/gnome/VncBox";

VncBoxService::VncBoxService(DBus::Connection& connection)
    : DBus::ObjectAdaptor(connection, VncBoxService::SERVICE_PATH)
{
}

int32_t
VncBoxService::RequestAccept(const std::map< std::string, std::string >& notifydata)
{
    return (int32_t) signal_request_accept(NotifyData(notifydata));
}

void
VncBoxService::NotifyAfterAccept(const std::map< std::string, std::string >& notifydata)
{
    signal_notify_after_accept(NotifyData(notifydata));
}

void
VncBoxService::NotifyGone(const std::map< std::string, std::string >& notifydata)
{
    signal_notify_gone(NotifyData(notifydata));
}
