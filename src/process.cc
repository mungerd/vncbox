/* process.cc
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

#include "process.h"

#include <sys/wait.h>
#include <signal.h>
#include <cerrno>
#include <iostream>

using Glib::ustring;


Process::ProcessMap Process::process_map;
Process::StringMap Process::common_env;

void
Process::poll_terminated()
{
    while (true) {
	int status;
	Glib::Pid pid = waitpid(-1, &status, WNOHANG);
	if (pid <= 0)
	    break;
	ProcessMap::iterator it = process_map.find(pid);
	if (it != process_map.end()) {
	    it->second->m_pid = 0;
	    it->second->signal_exit(status);
	    Glib::spawn_close_pid(pid);
	    process_map.erase(pid);
	}
	else
	    std::cerr << "WARNING: unknown child process " << pid << " terminated with status " << status << std::endl;
    }
}

bool
Process::running(Glib::Pid pid)
{
    return (pid > 0) && !(kill(pid, 0) < 0 && (errno == ESRCH));
}


Process::Process()
: working_dir("."), m_pid(0), m_input_channel(0), m_output_channel(0), m_error_channel(0)
{
}

Process::~Process()
{
    terminate();
}

void
Process::set_env(StringMap& env)
{
    env.insert(common_env.begin(), common_env.end());
}

void
Process::run(bool search_path, PipeFlags pipe_flags)
{
    if (running()) return;

    // default environment
    StringMap env;
    set_env(env);
    env.insert(override_env.begin(), override_env.end());

    // convert to VARIABLE=VALUE list
    std::list<ustring> envp;
    for (StringMap::const_iterator it = env.begin(); it != env.end(); it++)
        envp.push_back(it->first + "=" + it->second);

    Glib::SpawnFlags flags = Glib::SPAWN_DO_NOT_REAP_CHILD;
    if (search_path)
        flags |= Glib::SPAWN_SEARCH_PATH;

    int std_input, std_output, std_error;

    int* p_std_input    = pipe_flags & PIPE_INPUT  ? &std_input    : 0;
    int* p_std_output   = pipe_flags & PIPE_OUTPUT ? &std_output   : 0;
    int* p_std_error    = pipe_flags & PIPE_ERROR  ? &std_error    : 0;

    // start program
    Glib::spawn_async_with_pipes(working_dir,
        Glib::shell_parse_argv(get_command()),
        envp,
        flags,
        sigc::mem_fun(*this, &Process::prepare), &m_pid,
        p_std_input, p_std_output, p_std_error);

    process_map[m_pid] = this;

    if (pipe_flags & PIPE_INPUT)
        m_input_channel = Glib::IOChannel::create_from_fd(std_input);
    if (pipe_flags & PIPE_OUTPUT)
        m_output_channel = Glib::IOChannel::create_from_fd(std_output);
    if (pipe_flags & PIPE_ERROR)
        m_error_channel = Glib::IOChannel::create_from_fd(std_error);

#ifdef USE_GLIB_CHILD_WATCH
    Glib::signal_child_watch().connect(sigc::mem_fun(*this, &Process::on_child_watch), m_pid);
#endif
}

int
Process::wait()
{
    int status = -1;
    if (m_pid <= 0)
        return 0;

    waitpid(m_pid, &status, 0);
    Glib::spawn_close_pid(m_pid);
    process_map.erase(m_pid);
    m_pid = 0;

    // no need to close the IOChannels; they will close on unref if not already closed

    if (WIFEXITED(status))
        status = WEXITSTATUS(status);
    else if (WIFSIGNALED(status))
        throw ProcessError(ProcessError::TERMINATED_BY_SIGNAL, "process terminated by signal");
    else
        throw ProcessError(ProcessError::TERMINATED_UNKNOWN, "process terminated from unknown cause");

    return status;
}

int
Process::terminate()
{
    if (running())
        kill(m_pid, SIGTERM);
    int status;
    try {
        status = this->wait();
    }
    catch (ProcessError& e) {
        if (e.code != ProcessError::TERMINATED_BY_SIGNAL)
	    std::cerr << "WARNING: " << e.what() << std::endl;
	status = -1;
    }
    return status;
}

bool
Process::running() const
{
    return running(m_pid);
}

void
Process::on_child_watch(Glib::Pid pid, int priority)
{
    int status;
    try {
        status = wait();
    }
    catch (ProcessError& e) {
        // nothing to worry about
    }
    signal_exit(status);
}

void
Process::prepare()
{
    sigset_t newmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGINT);
    sigprocmask(SIG_BLOCK, &newmask, NULL);
}
