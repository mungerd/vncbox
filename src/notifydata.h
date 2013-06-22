/* notifydata.h
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

#ifndef __vncbox_NOTIFYDATA_H
#define __vncbox_NOTIFYDATA_H

#include <map>
#include <string>
#include <exception>
#include <time.h>

enum NotifyResponse
{
    NOTIFY_RESPONSE_ACCEPT      = 10,
    NOTIFY_RESPONSE_VIEWONLY    = 11,
    NOTIFY_RESPONSE_DENY        = 12
};

class NotifyDataError : public std::exception
{
public:
    enum Code
    {
        INVALID_BOOLEAN_VALUE,
        INVALID_NUMERIC_VALUE
    };

    const Code code;
    const std::string message;

    NotifyDataError(Code code_, std::string message_)
        : code(code_), message(message_) {}

    virtual ~NotifyDataError() throw() {}

    virtual const char* what() const throw()
        { return message.c_str(); }
};

struct NotifyData
{
public:
    std::string client_ip;
    std::string client_port;
    std::string server_ip;
    std::string server_port;
    long        x11vnc_pid;
    long        client_id;
    long        client_count;
    std::string mode;
    std::string state;
    long        login_viewonly;
    std::string username;
    time_t      login_time;
    time_t      current_time;

    NotifyData()
        : x11vnc_pid(0), client_id(0), client_count(0), login_viewonly(false) {}

    static const std::map<std::string, std::string> env_to_map();
    void load(const std::map<std::string, std::string>& map);

    NotifyData(const std::map<std::string, std::string>& map)
        : x11vnc_pid(0), client_id(0), client_count(0), login_viewonly(false)
        { load(map); }

private:
    static const std::string getenv_string(const std::string& var);
    static long getenv_long(const std::string& var);
    static bool getenv_bool(const std::string& var);

    static long string_to_long(const std::string& var);
    static unsigned long string_to_ulong(const std::string& var);
    static bool string_to_bool(const std::string& var);
};

#endif // __vncbox_NOTIFYDATA_H
