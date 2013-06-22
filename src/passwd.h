/* passwd.h
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

#ifndef __vncbox_PASSWD_H
#define __vncbox_PASSWD_H

#include <string>
#include <exception>
#include <unistd.h>

struct passwd;

class PasswdError : public std::exception {
public:
    const int no;
    const std::string message;

    PasswdError(int errno_);
    virtual ~PasswdError() throw() {}

    virtual const char* what() const throw()
        { return message.c_str(); }
};

struct Passwd
{
public:
    void load_current_uid();
    void load_current_euid();
    void load_uid(uid_t uid);
    void load_name(const std::string& name);
    void copy_from(struct passwd *pw);

    std::string name;
    std::string passwd;
    uid_t uid;
    gid_t gid;
    std::string gecos;
    std::string dir;
    std::string shell;
};

#endif // __vncbox_PASSWD_H
