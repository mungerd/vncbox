/* sysconf.h
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

#ifndef __vncbox_SYSCONF_H
#define __vncbox_SYSCONF_H

#include <glibmm.h>
#include <map>

class SysconfError : public Glib::Exception
{
public:
    enum Code {
        CANNOT_READ_FILE,
        WRITEABLE_BY_USER,
        APPLICATION_NOT_ALLOWED,
        INVALID_USER
    };

    const Code code;
    const Glib::ustring message;

    SysconfError(Code error_code, const Glib::ustring& error_message)
        : code(error_code), message(error_message) {}

    virtual ~SysconfError() throw() {}

    virtual Glib::ustring what() const { return message; }
};


class Sysconf
{
public:
    static const Glib::ustring FILENAME;

    Sysconf();

    const Glib::ustring& get_current_user() const
        { return m_current_user; }

    const Glib::ustring& get_anonymous_user() const
        { return m_anonymous_user; }

    bool is_application_allowed(const Glib::ustring& app) const;

    const Glib::ustring& get_command(const Glib::ustring& app) const;

    const Glib::ustring& get_vnc_server() const
        { return m_vnc_server; }

    const Glib::ustring& get_vnc_server_options() const
        { return m_vnc_server_options; }

    const Glib::ustring& get_notify() const
        { return m_notify; }

private:
    Glib::ustring m_current_user;
    Glib::ustring m_anonymous_user;
    std::map<Glib::ustring, Glib::ustring> m_allowed_applications;
    Glib::ustring m_vnc_server;
    Glib::ustring m_vnc_server_options;
    Glib::ustring m_notify;
};

#endif // __vncbox_SYSCONF_H
