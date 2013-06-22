/* setuidprocess.h
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

#ifndef __vncbox_SETUIDPROCESS_H
#define __vncbox_SETUIDPROCESS_H

#include "process.h"

class SetuidProcess : public Process
{
public:
    SetuidProcess(const Glib::ustring& command = "", uid_t uid = -1, gid_t gid = -1);
    virtual ~SetuidProcess();

    virtual const Glib::ustring get_command() const { return command; }

    Glib::ustring command;
    uid_t uid;
    gid_t gid;
    bool allow_root;  // set to true to allow uid/gid = 0

protected:
    virtual void set_env(StringMap& env);
    virtual void prepare();
};

#endif // __vncbox_SETUIDPROCESS_H
