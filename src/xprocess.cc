/* xprocess.cc
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

#include "xprocess.h"

#include <gdkmm.h>
#include <signal.h>

using Glib::ustring;


/* XServerProcess */

XServerProcess::XServerProcess(const Glib::ustring& binary, unsigned short display, const Glib::ustring& options, const Gdk::NativeWindow parent_id)
{
    this->binary = binary;
    this->display = display;
    this->options = options;
    this->parent_id = parent_id;
}

XServerProcess::~XServerProcess()
{
}

const ustring
XServerProcess::get_command() const
{
    ustring command = binary + " :" + ustring::format(display) + " " + options;
    if (parent_id)
        command += " -parent 0x" + ustring::format(std::hex, parent_id);
    return command;
}

void
XServerProcess::set_env(StringMap& env)
{
    Process::set_env(env);
    env["XAUTHORITY"] = Glib::getenv("XAUTHORITY");
    env["DISPLAY"] = Gdk::Display::get_default()->get_name();
}


void
XServerProcess::prepare()
{
    Process::prepare();
    // ask the X server to signal us when it accepts connections
    signal(SIGUSR1, SIG_IGN);
}

void
XServerProcess::run()
{
    // block SIGUSR1
    sigset_t newmask, oldmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &newmask, &oldmask);

    Process::run();

    // wait for SIGUSR1 from child X server
    int sig;
    sigwait(&newmask, &sig);

    // reset mask
    sigprocmask(SIG_SETMASK, &oldmask, NULL);
}


/* XClientProcess */

unsigned short
XClientProcess::display;

XClientProcess::XClientProcess(const Glib::ustring& command)
{
    this->command = command;
}

XClientProcess::~XClientProcess()
{
}

void
XClientProcess::set_env(StringMap& env)
{
    Process::set_env(env);
    env["XAUTHORITY"] = Glib::getenv("XAUTHORITY");
    env["DISPLAY"] = ":" + ustring::format(display);
}
