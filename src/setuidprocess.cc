/* setuidprocess.cc
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

#include "setuidprocess.h"

#include <sys/types.h>
#include <unistd.h>

using Glib::ustring;


SetuidProcess::SetuidProcess(const Glib::ustring& command, uid_t uid, gid_t gid)
{
    this->command = command;
    this->uid = uid;
    this->gid = gid;
    this->allow_root = false;
}

SetuidProcess::~SetuidProcess()
{
}

void
SetuidProcess::set_env(StringMap& env)
{
    Process::set_env(env);
    // TODO: maybe just use common_env -> delete this member function
    //env["XAUTHORITY"]   = Glib::getenv("XAUTHORITY");
    //env["DISPLAY"]      = Glib::getenv("DISPLAY");
}

void
SetuidProcess::prepare()
{
    Process::prepare();

    if ((uid == 0 || gid == 0) && !allow_root)
        throw ProcessError(ProcessError::OWNER_ERROR, "not allowed to run as root");

    if (uid >= 0) {
        if (setresuid(uid, uid, -1) != 0)
            throw ProcessError(ProcessError::OWNER_ERROR, "cannot set uid");
    }

    if (gid >= 0) {
        if (setresgid(gid, gid, -1) != 0)
            throw ProcessError(ProcessError::OWNER_ERROR, "cannot set gid");
    }
}
