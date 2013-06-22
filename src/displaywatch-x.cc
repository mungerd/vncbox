/* displaywatch-x.cc
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

#include "displaywatch.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <time.h>

// TODO: Glib::RefPtr<Gdk::Pixbuf> get_icon(int ideal_width, int ideal_height) const

/* Part of this code was adapted/inspired from libwnck by
 * Havoc Pennington and Vincent Untz.
 */

/* class WindowInfo */

using Glib::ustring;

const Glib::ustring
DisplayWatch::WindowInfo::NO_NAME_STRING("(no name)");



ustring
DisplayWatch::WindowInfo::get_text_property(Atom property) const
{
    ustring retval;

    gdk_error_trap_push();

    XTextProperty text;
    text.nitems = 0;

    if (!XGetTextProperty(
        get_xdisplay(),
        get_xwindow(),
        &text,
        property)) {
        throw DisplayWatchError(DisplayWatchError::PROPERTY_NOT_FOUND,
            "cannot find text property");
    }


    // TODO
    gchar** list;
    int nitems = gdk_text_property_to_utf8_list_for_display(
        get_gdk_display(),
        gdk_x11_xatom_to_atom_for_display(get_gdk_display(), text.encoding),
        text.format,
        text.value,
        text.nitems,
        &list);

    if (text.value)
        XFree(text.value);

    if (nitems < 1)
        throw DisplayWatchError(DisplayWatchError::PROPERTY_NOT_FOUND,
            "cannot convert text property to UTF-8");

    retval = list[0];
    g_strfreev(list);

    gdk_error_trap_pop();

    return retval;
}

ustring
DisplayWatch::WindowInfo::get_utf8_property(const ustring& property_name) const
{
    Atom property = get_xatom(property_name);
    Atom utf8_string = get_xatom("UTF8_STRING");

    gdk_error_trap_push();
    Atom type = None;
    int format;
    unsigned long nitems, bytes_after;
    unsigned char* val = NULL;
    int result = XGetWindowProperty(
        get_xdisplay(),
        get_xwindow(),
        property,
        0, G_MAXLONG,
        False,
        utf8_string,
        &type, &format, &nitems, &bytes_after, &val);
    int err = gdk_error_trap_pop();

    if (err != Success || result != Success)
        throw DisplayWatchError(DisplayWatchError::PROPERTY_NOT_FOUND,
            "cannot find UTF-8 property " + property_name);

    if (type != utf8_string || format != 8 || nitems == 0) {
        if (val) XFree(val);
        throw DisplayWatchError(DisplayWatchError::PROPERTY_NOT_FOUND,
            "cannot find UTF-8 property " + property_name);
    }

    ustring retval((char*) val, nitems);
    XFree(val);

    return retval;
}

ustring
DisplayWatch::WindowInfo::get_name() const
{
    try { return get_utf8_property("_NET_WM_VISIBLE_NAME"); }
    catch (DisplayWatchError& e) {}
    try { return get_utf8_property("_NET_WM_NAME"); }
    catch (DisplayWatchError& e) {}
    try { return get_text_property(XA_WM_NAME); }
    catch (DisplayWatchError& e) {}
    return NO_NAME_STRING;
}

ustring
DisplayWatch::WindowInfo::get_icon_name() const
{
    try { return get_utf8_property("_NET_WM_VISIBLE_ICON_NAME"); }
    catch (DisplayWatchError& e) {}
    try { return get_utf8_property("_NET_WM_ICON_NAME"); }
    catch (DisplayWatchError& e) {}
    try { return get_text_property(XA_WM_ICON_NAME); }
    catch (DisplayWatchError& e) {}
    return NO_NAME_STRING;
}


void
DisplayWatch::WindowInfo::activate() const
{
    XEvent ev;

    ev.xclient.type = ClientMessage;
    ev.xclient.serial = 0;
    ev.xclient.send_event = True;
    ev.xclient.display = get_xdisplay();
    ev.xclient.window = get_xwindow();
    ev.xclient.message_type = gdk_x11_get_xatom_by_name_for_display(
        m_window->get_display()->gobj(), "_NET_ACTIVE_WINDOW");
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = 2; // 1: application; 2: pager/user action
    ev.xclient.data.l[1] = time(NULL);
    ev.xclient.data.l[2] = 0;
    ev.xclient.data.l[3] = 0;
    ev.xclient.data.l[4] = 0;

    gdk_error_trap_push();
    XSendEvent(
        get_xdisplay(),
        get_xroot(),
        False,
        SubstructureRedirectMask | SubstructureNotifyMask,
        &ev);
    gdk_error_trap_pop();
}
