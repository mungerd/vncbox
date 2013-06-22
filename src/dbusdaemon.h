/* dbusdaemon.h
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

#ifndef __vncbox_DBUS_DAEMON_H
#define __vncbox_DBUS_DAEMON_H

#include "process.h"

class DBusDaemon {
public:
    DBusDaemon();
    virtual ~DBusDaemon();

    const Glib::ustring& get_address() const { return m_address; }

    static const Glib::ustring random_string(size_t length);

protected:
    CommandLineProcess m_dbus_process;
    Glib::ustring m_address;
};

#endif // __vncbox_DBUS_DAEMON_H
