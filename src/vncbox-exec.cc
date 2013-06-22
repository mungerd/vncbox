/* vncbox-exec.cc
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sysconf.h"
#include "profile.h"
#include "passwd.h"
#include "setuidprocess.h"
#include "xauthxx.h"
#include "notifydata.h"

#include <glibmm.h>
#include <iostream>
#include <signal.h>

#include <map>

using Glib::ustring;

class VncBoxExecOptionGroup : public Glib::OptionGroup
{
public:
    VncBoxExecOptionGroup();
    int display;
    ustring xauth_add_cookie;
    ustring xauth_remove_cookie;
    ustring dbus_address;
    ustring server_key;
    Glib::OptionGroup::vecustrings remaining;
};

static volatile Glib::Pid child_pid = 0;

static void signals_catch(int sig);
static void signals_setup();
static int remove_xauth_entry(const Passwd& pw, const ustring& cookie);
static int add_xauth_entry(const Passwd& pw, int display, const ustring& cookie);
static const ustring get_xauth_filename(const Passwd& pw);
static const ustring escape_quotes(const ustring& str);
static const ustring get_vncbox_notify();


int
main(int argc, char *argv[])
{
    Glib::OptionContext context;
    VncBoxExecOptionGroup options;
    context.set_main_group(options);

    try {
        context.parse(argc, argv);
    }
    catch (const Glib::Error& e) {
        std::cout << e.what() << std::endl;
    }

    SetuidProcess process;

    try {
        Sysconf sysconf;

        Passwd pw;
        pw.load_name(sysconf.get_anonymous_user());

        if (pw.uid == 0) {
            std::cerr << "do not use root as anonymous user" << std::endl;
            return -1;
        }

        if (!options.xauth_remove_cookie.empty()) {
            return remove_xauth_entry(pw, options.xauth_remove_cookie);
        }
        if (!options.xauth_add_cookie.empty()) {
            return add_xauth_entry(pw, options.display, options.xauth_add_cookie);
        }

        if (options.remaining.size() != 1) {
            std::cerr << "no application name provided" << std::endl;
            return -1;
        }

        ustring application = options.remaining[0];
        ustring command;

        std::map<ustring, ustring> env;

        if (options.display >= 0)
            env["DISPLAY"] = ":" + ustring::format(options.display);

        if (application == "vnc-server") {
            if (options.dbus_address.empty()
                || options.server_key.empty()) {
                std::cerr << "options dbus-address and server-key must be given" << std::endl;
                return -1;
            }

            Profile profile;
            try {
                profile.read_from(Glib::IOChannel::create_from_fd(0));
            }
            catch (Glib::Exception& e) {
                std::cerr << "error receiving profile information: " << e.what() << std::endl;
            }

            std::cout << "ports: " << profile.rfb_port << " / " << profile.http_port << std::endl;

            command = sysconf.get_vnc_server();
            env["DBUS_SESSION_BUS_ADDRESS"] = options.dbus_address;
            env["VNBOX_SERVER_KEY"] = options.server_key;
            env["PATH"] = "/usr/bin:/bin";
            command += ustring::compose(" -accept 'yes:%1,no:*,view:%2 %3'",
                NOTIFY_RESPONSE_ACCEPT, NOTIFY_RESPONSE_VIEWONLY, get_vncbox_notify());
            command += ustring::compose(" -afteraccept '%1'", get_vncbox_notify());
            command += ustring::compose(" -gone '%1'", get_vncbox_notify());

            if (!profile.password.empty())
                command += " -passwd " + ustring::format(profile.password);

            command += " -rfbport " + ustring::format(profile.rfb_port);
            if (profile.rfb_ssl) {
                command += " -ssl";
                if (!profile.ssl_cert.empty())
                    command += " " + profile.ssl_cert;
            }

            if (profile.http) {
                if (profile.http_ssl)
                    command += " -http_ssl";
                else
                    command += " -http";

                command += " -httpport " + ustring::format(profile.http_port);
            }

            // misc options dictionary
            if (!profile.vnc_options.empty())
                command += " " + profile.vnc_options;

            // system options
            if (!sysconf.get_vnc_server_options().empty())
                command += " " + sysconf.get_vnc_server_options().empty();

        }
        else {
            command = sysconf.get_command(application);
        }

        // using user's shell and piping the command into it allows the
        // application to be jailed if the shell does a chroot
        process.working_dir = pw.dir;
        process.command = pw.shell;
        process.uid = pw.uid;
        process.gid = pw.gid;

        signals_setup();

        process.run(false, Process::PIPE_INPUT);
        child_pid = process.get_pid();

        process.get_input_channel()->set_encoding();

        for (std::map<ustring, ustring>::const_iterator it = env.begin();
            it != env.end(); it++) {
            process.get_input_channel()->write("export " + it->first + "='"
                + escape_quotes(it->second) + "'\n");
        }

        process.get_input_channel()->write("exec " + command + "\n");
        process.get_input_channel()->close();
    }
    catch (const Glib::Exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    int status;
    try {
        status = process.wait();
    }
    catch (ProcessError& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return status;
}

static int
remove_xauth_entry(const Passwd& pw, const ustring& cookie)
{
    // remove entry from Xauthority file
    ustring filename = get_xauth_filename(pw);

    if (setresuid(pw.uid, pw.uid, -1) != 0) {
        std::cerr << "cannot change uid to remove Xauthority entry" << std::endl;
        return -1;
    }

    Xau::XauthList auth_list;
    try {
        auth_list.load_from_file(filename);
    }
    catch (Xau::Error& e) {
        // nothing to worry about
    }
    auth_list.remove(Xau::MagicCookie(cookie));
    auth_list.write_to_file(filename);

    return 0;
}

static int
add_xauth_entry(const Passwd& pw, int display, const ustring& cookie)
{
    if (display < 0) {
        std::cerr << "a display number must be specified" << std::endl;
        return -1;
    }

    // add entry to Xauthority file
    ustring filename = get_xauth_filename(pw);

    if (setresuid(pw.uid, pw.uid, -1) != 0) {
        std::cerr << "cannot change uid to add Xauthority entry" << std::endl;
        return -1;
    }

    Xau::XauthList auth_list;
    try {
        auth_list.load_from_file(filename);
    }
    catch (Xau::Error& e) {
        // nothing to worry about
    }
    auth_list.push_back(Xau::Xauth(
        Xau::LocalAddress(),
        Xau::Display(display),
        Xau::MagicCookie(cookie)));
    auth_list.write_to_file(filename);

    return 0;

#if 0
    // parse display name
    ustring address = display_name.substr(0, display_name.rfind(':'));
    ustring display = display_name.substr(display_name.rfind(':') + 1);


    if (address.empty())
        auth.address = Xau::LocalAddress();
    else if (Glib::Regex::match_simple("[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}", address))
        auth.address = Xau::InternetAddress(address);
    else
        auth.address = Xau::LocalAddress(address);
#endif
}

static const
ustring get_xauth_filename(const Passwd& pw)
{
    return pw.dir + G_DIR_SEPARATOR_S + ".Xauthority";
}

VncBoxExecOptionGroup::VncBoxExecOptionGroup()
: Glib::OptionGroup("vncbox-exec", "VNC Box exec", "VNC Box exec options")
{
    display = -1;

    Glib::OptionEntry entry1;
    entry1.set_long_name("display");
    entry1.set_short_name('d');
    entry1.set_description("X display number (without the ':')");
    add_entry(entry1, display);

    Glib::OptionEntry entry2;
    entry2.set_long_name("add-xauth");
    entry2.set_short_name('a');
    entry2.set_description("add Xauthority cookie");
    add_entry(entry2, xauth_add_cookie);

    Glib::OptionEntry entry3;
    entry3.set_long_name("remove-xauth");
    entry3.set_short_name('r');
    entry3.set_description("remove Xauthority cookie");
    add_entry(entry3, xauth_remove_cookie);

    Glib::OptionEntry entry4;
    entry4.set_long_name("dbus-address");
    entry4.set_short_name('b');
    entry4.set_description("D-Bus address");
    add_entry(entry4, dbus_address);

    Glib::OptionEntry entry5;
    entry5.set_long_name("server-key");
    entry5.set_short_name('k');
    entry5.set_description("VNC Box server key");
    add_entry(entry5, server_key);

    Glib::OptionEntry entry_remaining;
    entry_remaining.set_long_name(G_OPTION_REMAINING);
    entry_remaining.set_arg_description(G_OPTION_REMAINING);

    add_entry(entry_remaining, remaining);
}

static void
signals_catch(int sig)
{
    if (child_pid) kill(child_pid, sig);
}

static void
signals_setup()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = signals_catch;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
}

static const ustring
escape_quotes(const ustring& str)
{
    ustring escaped_str;
    for (ustring::const_iterator it = str.begin(); it != str.end(); it++) {
        if (*it == '\'') escaped_str += '\\';
        escaped_str += *it;
    }
    return escaped_str;
}

static const ustring
get_vncbox_notify()
{
    return LIBEXECDIR G_DIR_SEPARATOR_S "vncbox-notify";
}
