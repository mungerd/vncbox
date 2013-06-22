/* dbusdaemon.cc
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

#include "dbusdaemon.h"
#include "dbus-config.h"
#include "sysconf.h"

#include <stdio.h>
#include <sys/stat.h>


DBusDaemon::DBusDaemon()
{
    Sysconf sysconf;

    // create config file
    std::string tmp_filename;
    Glib::RefPtr<Glib::IOChannel> tmp_file = Glib::IOChannel::create_from_fd(
        Glib::file_open_tmp(tmp_filename));
    chmod(tmp_filename.c_str(), S_IRUSR | S_IWUSR);
    tmp_file->write(Glib::ustring::compose(VNCBOX_DBUS_CONFIG_STRING,
        random_string(6), sysconf.get_current_user(), sysconf.get_anonymous_user()));
    tmp_file->close();

    // start daemon
    m_dbus_process.command_line =
        "dbus-daemon --print-address --config-file=" + tmp_filename;
    m_dbus_process.run(true, Process::PIPE_OUTPUT);

    // read bus address
    Glib::RefPtr<Glib::IOChannel> output = m_dbus_process.get_output_channel();
    output->read_line(m_address);
    output->close();
    m_address = m_address.substr(0, m_address.length() - 1);

    // remove config file
    remove(tmp_filename.c_str());
}

DBusDaemon::~DBusDaemon()
{
    m_dbus_process.terminate();
}

const Glib::ustring
DBusDaemon::random_string(size_t length)
{
    Glib::ustring matrix("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    Glib::Rand rand;
    Glib::ustring str;

    while (str.length() < length)
        str += matrix[rand.get_int_range(0, matrix.length() - 1)];

    return str;
}
