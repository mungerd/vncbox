/* xprocess.h
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

#ifndef __vncbox_XPROCESS_H
#define __vncbox_XPROCESS_H

#include "process.h"
#include <gdkmm.h>

class XServerProcess : public Process
{
public:
    XServerProcess(const Glib::ustring& binary = "", unsigned short display = 0, const Glib::ustring& options = "", const Gdk::NativeWindow parent_id = 0);
    virtual ~XServerProcess();

    virtual void run();
    virtual const Glib::ustring get_command() const;

    Glib::ustring binary;
    Glib::ustring options;
    unsigned short display;
    Gdk::NativeWindow parent_id;

protected:
    virtual void prepare();
    virtual void set_env(StringMap& env);
};

class XClientProcess : public Process
{
public:
    XClientProcess(const Glib::ustring& command = "");
    virtual ~XClientProcess();

    virtual const Glib::ustring get_command() const { return command; }

    Glib::ustring command;
    static unsigned short display; // all clients share the same display

protected:
    virtual void set_env(StringMap& env);
};

#endif // __vncbox_XPROCESS_H
