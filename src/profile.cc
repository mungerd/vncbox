/* profile.cc
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

#include "profile.h"
#include "serializer.h"
#include "randomport.h"

#include <iostream>

using Glib::ustring;


Profile::Profile(const ustring& name)
{
	preferred_display   = 1;
	window_width        = 320;
	window_height       = 200;
	rfb_port            = 0;
	rfb_ssl             = false;
	http                = false;
	http_port           = 0;
	http_ssl            = false;
	password            = "";

	if (!name.empty())
        load(name);
}

void
Profile::load(const Glib::ustring& name)
{
    ustring filename = Glib::get_user_config_dir() + G_DIR_SEPARATOR_S
        + "vncbox" + G_DIR_SEPARATOR_S + name + PROFILE_EXTENSION;

    Glib::KeyFile keyfile;

    try {
        keyfile.load_from_file(filename, Glib::KEY_FILE_NONE);
    }
    catch (Glib::Exception& e) {
        throw ProfileError(ProfileError::CANNOT_READ_FILE,
            "cannot read profile file " + filename + ": " + e.what());
    }

    // X server
    preferred_display   = keyfile.get_integer("X server", "preferred display");
    window_width 	    = keyfile.get_integer("X server", "width");
    window_height 	    = keyfile.get_integer("X server", "height");
    xserver_options     = keyfile.get_string("X server", "xserver options");

    // VNC: RFB
    try {
        if (keyfile.get_string("VNC", "RFB port") == "RANDOM")
            rfb_port = get_random_inet_port();
        else
            rfb_port    = keyfile.get_integer("VNC", "RFB port");
    }
    catch (RandomPortError& e) {
        std::cerr << "error selecting a random RFB port: " << e.what()
            << "; using " << rfb_port << std::endl;
    }
    rfb_ssl = keyfile.get_boolean("VNC", "RFB SSL");
    try {
        ssl_cert = keyfile.get_string("VNC", "SSL cert");
    }
    catch (Glib::KeyFileError& e) {
        // just leave ssl_cert empty
    }

    // VNC: HTTP
    http = keyfile.get_boolean("VNC", "HTTP");
    if (http) {
        try {
            if (keyfile.get_string("VNC", "HTTP port") == "RANDOM")
                http_port = get_random_inet_port();
            else
                http_port   = keyfile.get_integer("VNC", "HTTP port");
        }
        catch (RandomPortError& e) {
            std::cerr << "error selecting a random HTTP port: " << e.what()
                << "; using " << http_port << std::endl;
        }
        http_ssl = keyfile.get_boolean("VNC", "HTTP SSL");
    }

    // password
    try {
        password = keyfile.get_string("VNC", "password");
    }
    catch (Glib::KeyFileError& e) {
        // just leave it empty
    }
    if (password == "RANDOM")
        password = random_string(6);

    // use UPnP
    upnp = keyfile.get_boolean("VNC", "UPnP");

    // misc options
    try {
        vnc_options = keyfile.get_string("VNC", "options");
    }
    catch (Glib::KeyFileError& e) {
        // just leave it empty
    }


    // applications
    std::list<ustring> apps = keyfile.get_string_list("X server", "self applications");
    for (std::list<ustring>::iterator it = apps.begin(); it != apps.end(); it++) {
        ustring cmd = keyfile.get_string("application " + *it, "command");
        self_applications[*it] = cmd;
    }
    anonymous_applications = keyfile.get_string_list("X server", "anonymous applications");
    startup_applications = keyfile.get_string_list("X server", "startup applications");

    // applications titles
    for (std::map<ustring,ustring>::iterator it = self_applications.begin();
        it != self_applications.end(); it++) {
        ustring title;
        try {
            title = keyfile.get_string("application " + it->first, "title");
        }
        catch (Glib::KeyFileError& e) {
            title = it->first;
        }
        applications_titles[it->first] = "!" + title;
    }
    for (std::list<ustring>::iterator it = anonymous_applications.begin();
        it != anonymous_applications.end(); it++) {
        ustring title;
        try {
            title = keyfile.get_string("application " + *it, "title");
        }
        catch (Glib::KeyFileError& e) {
            title = *it;
        }
        applications_titles[*it] = title;
    }
}

void
Profile::write_to(Glib::RefPtr<Glib::IOChannel> output_channel) const
{
    Serializer serializer(output_channel);

    serializer.write(preferred_display);
    serializer.write(window_width);
    serializer.write(window_height);
    serializer.write(xserver_options);

    serializer.write(applications_titles);
    serializer.write(self_applications);
    serializer.write(anonymous_applications);
    serializer.write(startup_applications);

    serializer.write(rfb_port);
    serializer.write(rfb_ssl);
    serializer.write(ssl_cert);

    serializer.write(http);
    serializer.write(http_port);
    serializer.write(http_ssl);

    serializer.write(vnc_options);
    serializer.write(password);
    serializer.write(upnp);
}

void
Profile::read_from(Glib::RefPtr<Glib::IOChannel> input_channel)
{
    Serializer serializer(input_channel);

    serializer.read(preferred_display);
    serializer.read(window_width);
    serializer.read(window_height);
    serializer.read(xserver_options);

    serializer.read(applications_titles);
    serializer.read(self_applications);
    serializer.read(anonymous_applications);
    serializer.read(startup_applications);

    serializer.read(rfb_port);
    serializer.read(rfb_ssl);
    serializer.read(ssl_cert);

    serializer.read(http);
    serializer.read(http_port);
    serializer.read(http_ssl);

    serializer.read(vnc_options);
    serializer.read(password);
    serializer.read(upnp);
}

const ustring
Profile::random_string(size_t length)
{
    ustring matrix("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    Glib::Rand rand;
    ustring str;

    while (str.length() < length)
        str += matrix[rand.get_int_range(0, matrix.length() - 1)];

    return str;
}
