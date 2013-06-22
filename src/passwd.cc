/* passwd.cc
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

#include "passwd.h"

#include <sys/types.h>
#include <pwd.h>
#include <cerrno>
#include <cstring>

#include <string>

using std::string;


void
Passwd::copy_from(struct passwd *pw)
{
    name    = pw->pw_name;
    passwd  = pw->pw_passwd;
    uid     = pw->pw_uid;
    gid     = pw->pw_gid;
    gecos   = pw->pw_gecos;
    dir     = pw->pw_dir;
    shell   = pw->pw_shell;
}

void
Passwd::load_current_uid()
{
    load_uid(getuid());
}

void
Passwd::load_current_euid()
{
    load_uid(geteuid());
}

void
Passwd::load_uid(uid_t uid) {
    errno = 0;
    struct passwd *pw = getpwuid(uid);
    if (pw == NULL) throw PasswdError(errno);
    copy_from(pw);
}

void
Passwd::load_name(const string& name) {
    errno = 0;
    struct passwd *pw = getpwnam(name.c_str());
    if (pw == NULL) throw PasswdError(errno);
    copy_from(pw);
}

PasswdError::PasswdError(int errno_)
    : no(errno_),
      message(std::strerror(errno_))
{
}
