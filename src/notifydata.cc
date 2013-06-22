/* notifydata.cc
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

#include "notifydata.h"
#include <cstdlib>
#include <cerrno>

#define ENV_TO_MAP(var) map[#var] = getenv_string(#var)

#undef NOTIFYDATA_USE_EXCEPTIONS

const std::map<std::string, std::string>
NotifyData::env_to_map()
{
    std::map<std::string, std::string> map;
    ENV_TO_MAP(RFB_CLIENT_IP);
    ENV_TO_MAP(RFB_CLIENT_PORT);
    ENV_TO_MAP(RFB_SERVER_IP);
    ENV_TO_MAP(RFB_SERVER_PORT);
    ENV_TO_MAP(RFB_X11VNC_PID);
    ENV_TO_MAP(RFB_CLIENT_ID);
    ENV_TO_MAP(RFB_CLIENT_COUNT);
    ENV_TO_MAP(RFB_MODE);
    ENV_TO_MAP(RFB_STATE);
    ENV_TO_MAP(RFB_LOGIN_VIEWONLY);
    ENV_TO_MAP(RFB_USERNAME);
    ENV_TO_MAP(RFB_LOGIN_TIME);
    ENV_TO_MAP(RFB_CURRENT_TIME);
    return map;
}

void
NotifyData::load(const std::map<std::string, std::string>& map)
{
    client_ip       = map.find("RFB_CLIENT_IP")->second;
    client_port     = map.find("RFB_CLIENT_PORT")->second;
    server_ip       = map.find("RFB_SERVER_IP")->second;
    server_port     = map.find("RFB_SERVER_PORT")->second;

    x11vnc_pid      = string_to_long(map.find("RFB_X11VNC_PID")->second);
    client_id       = string_to_long(map.find("RFB_CLIENT_ID")->second);
    client_count    = string_to_long(map.find("RFB_CLIENT_COUNT")->second);

    mode            = map.find("RFB_MODE")->second;
    state           = map.find("RFB_STATE")->second;

    login_viewonly  = string_to_long(map.find("RFB_LOGIN_VIEWONLY")->second);

    username        = map.find("RFB_USERNAME")->second;
    login_time      = string_to_ulong(map.find("RFB_LOGIN_TIME")->second);
    current_time    = string_to_ulong(map.find("RFB_CURRENT_TIME")->second);
}

const std::string
NotifyData::getenv_string(const std::string& var)
{
    const char* val = std::getenv(var.c_str());
    return std::string(val ? val : "");
}

long
NotifyData::string_to_long(const std::string& strval)
{
#ifdef NOTIFYDATA_USE_EXCEPTIONS
    errno = 0;
#endif
    long val = std::strtol(strval.c_str(), NULL, 0);
#ifdef NOTIFYDATA_USE_EXCEPTIONS
    if (errno != 0)
        throw NotifyDataError(NotifyDataError::INVALID_NUMERIC_VALUE,
            "invalid numeric value: " + val);
#endif

    return val;
}

unsigned long
NotifyData::string_to_ulong(const std::string& strval)
{
#ifdef NOTIFYDATA_USE_EXCEPTIONS
    errno = 0;
#endif
    long val = std::strtoul(strval.c_str(), NULL, 0);
#ifdef NOTIFYDATA_USE_EXCEPTIONS
    if (errno != 0)
        throw NotifyDataError(NotifyDataError::INVALID_NUMERIC_VALUE,
            "invalid numeric value: " + val);
#endif

    return val;
}

long
NotifyData::getenv_long(const std::string& var)
{
    return string_to_long(getenv_string(var));
}

bool
NotifyData::string_to_bool(const std::string& strval)
{
#ifdef NOTIFYDATA_USE_EXCEPTIONS
    long longval;
    try {
        longval = string_to_long(strval);
    }
    catch (NotifyDataError& e) {
        throw NotifyDataError(NotifyDataError::INVALID_BOOLEAN_VALUE,
            "invalid boolean value: " + strval);
    }

    if (longval == 0)
        return false;
    else if (longval == 1)
        return true;
    else
        throw NotifyDataError(NotifyDataError::INVALID_BOOLEAN_VALUE,
            "invalid boolean value: " + strval);
#else
    return (bool) string_to_long(strval);
#endif
}

bool
NotifyData::getenv_bool(const std::string& var)
{
    return string_to_bool(var);
}
