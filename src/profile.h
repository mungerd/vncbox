/* profile.h
 *
 * Copyright (C) 2008, 2013  David Munger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General License for more details.
 *
 * You should have received a copy of the GNU General License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 * 	David Munger <mungerd@gmail.com>
 */

#ifndef __vncbox_PROFILE_H
#define __vncbox_PROFILE_H

#define PROFILE_EXTENSION ".profile"

#include <glibmm.h>
#include <list>
#include <map>

class ProfileError : public Glib::Exception
{
public:
    enum Code {
        CANNOT_READ_FILE
    };

    const Code code;
    const Glib::ustring message;

    ProfileError(Code error_code, const Glib::ustring& error_message)
        : code(error_code), message(error_message) {}

    virtual ~ProfileError() throw() {}

    virtual Glib::ustring what() const { return message; }
};

class Profile
{
public:

	// X server parameters
    unsigned short preferred_display;
    unsigned short window_width;
    unsigned short window_height;
    Glib::ustring xserver_options;

    // applications
    std::map<Glib::ustring, Glib::ustring>  applications_titles;
    std::map<Glib::ustring, Glib::ustring>  self_applications;
    std::list<Glib::ustring>                anonymous_applications;
    std::list<Glib::ustring>                startup_applications;

    // VNC server parameters
    unsigned short  rfb_port;
    bool            rfb_ssl;
    Glib::ustring   ssl_cert;

    bool            http;
    unsigned short  http_port;
    bool            http_ssl;

    Glib::ustring   vnc_options;
    Glib::ustring   password;
    bool            upnp;

	// constructors
	Profile(const Glib::ustring& name = "");

    // methods
    void load(const Glib::ustring& name);

    void write_to(Glib::RefPtr<Glib::IOChannel> output_channel) const;
    void read_from(Glib::RefPtr<Glib::IOChannel> input_channel);

protected:
    static const Glib::ustring random_string(size_t length);
};

#endif // __vncbox_PROFILE_H
