/* process.h
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

#ifndef __vncbox_PROCESS_H
#define __vncbox_PROCESS_H

#include <glibmm.h>
#include <map>

class ProcessError : public Glib::Exception
{
public:
    enum Code {
        TERMINATED_BY_SIGNAL,
        TERMINATED_UNKNOWN,
        OWNER_ERROR
    };

    const Code code;
    const Glib::ustring message;

    ProcessError(Code error_code, const Glib::ustring& error_message)
        : code(error_code), message(error_message) {}

    virtual ~ProcessError() throw() {}

    virtual Glib::ustring what() const { return message; }
};


class Process
{
public:
    enum PipeFlags {
        PIPE_NONE   = 0,
        PIPE_INPUT  = 1 << 0,
        PIPE_OUTPUT = 1 << 1,
        PIPE_ERROR  = 1 << 2
    };

    typedef std::map<Glib::ustring, Glib::ustring> StringMap;
    static bool running(Glib::Pid pid);
    static void poll_terminated();
    static StringMap common_env;
    StringMap override_env;

    virtual ~Process();
    virtual void run(bool search_path = true, PipeFlags pipe_flags = PIPE_NONE);
    virtual int wait();
    virtual int terminate();

    bool running() const;

    Glib::Pid get_pid() const
        { return m_pid; }

    virtual const Glib::ustring get_command() const = 0;

    sigc::signal<void,int> signal_exit;
    Glib::ustring working_dir;

    Glib::RefPtr<Glib::IOChannel> get_input_channel()  { return m_input_channel; }
    Glib::RefPtr<Glib::IOChannel> get_output_channel() { return m_output_channel; }
    Glib::RefPtr<Glib::IOChannel> get_error_channel()  { return m_error_channel; }

protected:
    Process();
    virtual void prepare();
    virtual void set_env(StringMap& env);
    virtual void on_child_watch(Glib::Pid pid, int priority);

    Glib::Pid m_pid;
    Glib::ustring m_working_dir;

    Glib::RefPtr<Glib::IOChannel> m_input_channel;
    Glib::RefPtr<Glib::IOChannel> m_output_channel;
    Glib::RefPtr<Glib::IOChannel> m_error_channel;

    typedef std::map<Glib::Pid, Process*> ProcessMap;
    static ProcessMap process_map;
};

class CommandLineProcess : public Process
{
public:
    CommandLineProcess(const Glib::ustring& command = "")
        : command_line(command) {}
    virtual const Glib::ustring get_command() const
        { return command_line; }
    Glib::ustring command_line;
};

#endif // __vncbox_PROCESS_H
