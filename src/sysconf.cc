/* sysconf.cc
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

#include "sysconf.h"
#include "passwd.h"
#include <unistd.h>

#include <list>

using Glib::ustring;

const ustring
Sysconf::FILENAME = SYSCONFDIR G_DIR_SEPARATOR_S "vncbox.conf";

Sysconf::Sysconf()
{
    // check that file is not writeable by user
    if (access(FILENAME.c_str(), W_OK) == 0)
        throw SysconfError(SysconfError::WRITEABLE_BY_USER,
            "system configuration file " + FILENAME + " is writeable by user");

    // read file
    Glib::KeyFile keyfile;

    try {
        keyfile.load_from_file(FILENAME, Glib::KEY_FILE_NONE);
    }
    catch (Glib::Exception& e) {
        throw SysconfError(SysconfError::CANNOT_READ_FILE,
            "cannot read system configuration file " + FILENAME + ": " + e.what());
    }

    Passwd passwd;
    try {
        passwd.load_current_uid();
    }
    catch (PasswdError& e) {
        throw SysconfError(SysconfError::INVALID_USER,
            ustring("cannot find current user name - ") + e.what());
    }
    m_current_user = passwd.name;

    ustring user_section = "user " + m_current_user;
    m_anonymous_user = keyfile.get_string(user_section, "anonymous user");
    try {
        passwd.load_name(m_anonymous_user);
    }
    catch (PasswdError& e) {
        throw SysconfError(SysconfError::INVALID_USER,
            "invalid anonymous user " + m_anonymous_user + ": " + e.what());
    }

    std::list<ustring> apps = keyfile.get_string_list(user_section, "allowed applications");
    for (std::list<ustring>::iterator it = apps.begin(); it != apps.end(); it++) {
        ustring cmd = keyfile.get_string("application " + *it, "command");
        m_allowed_applications[*it] = cmd;
    }

    ustring anonymous_user_section = "anonymous user " + m_anonymous_user;
    m_vnc_server    = keyfile.get_string(anonymous_user_section, "vnc server");
    m_notify        = keyfile.get_string(anonymous_user_section, "notify");

    try {
        m_vnc_server_options = keyfile.get_string("VNC", "vnc server options");
    }
    catch (Glib::KeyFileError& e) {
        // just leave it empty
    }
}

bool
Sysconf::is_application_allowed(const ustring& app) const
{
    return m_allowed_applications.find(app) != m_allowed_applications.end();
}

const ustring&
Sysconf::get_command(const ustring& app) const
{
    if (!is_application_allowed(app))
        throw SysconfError(SysconfError::APPLICATION_NOT_ALLOWED,
            "application " + app + " is not allowed");

    return m_allowed_applications.find(app)->second;
}
