/* displaywatch.h
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

#ifndef __vncbox_DISPLAY_WATCH_H
#define __vncbox_DISPLAY_WATCH_H

#include <glibmm.h>
#include <gdkmm.h>
#include <list>

#include <gdk/gdkx.h>
#include <X11/X.h>


class DisplayWatchError : public Glib::Exception
{
public:
    enum Code {
        INVALID_DISPLAY_NAME,
        PROPERTY_NOT_FOUND
    };

    const Code code;
    const Glib::ustring message;

    DisplayWatchError(Code error_code, const Glib::ustring& error_message)
        : code(error_code), message(error_message) {}

    virtual ~DisplayWatchError() throw() {}

    virtual Glib::ustring what() const { return message; }
};

class DisplayWatch
{
public:

    class WindowInfo
    {
    protected:
        Glib::RefPtr<Gdk::Window> m_window;
    public:

        static const Glib::ustring NO_NAME_STRING;

        WindowInfo(Glib::RefPtr<Gdk::Window> w)
            : m_window(w) {}

        Glib::RefPtr<Gdk::Window> get_window() const
            { return m_window; }

        Glib::ustring get_name() const;
        Glib::ustring get_icon_name() const;

        void activate() const;

    protected:
        Window get_xwindow() const
            { return GDK_WINDOW_XWINDOW(m_window->gobj()); }

        Window get_xroot() const
            { return GDK_WINDOW_XWINDOW(m_window->get_screen()->get_root_window()->gobj()); }

        Screen* get_xscreen() const
            { return GDK_SCREEN_XSCREEN(m_window->get_screen()->gobj()); }

        Display* get_xdisplay() const
            { return GDK_DISPLAY_XDISPLAY(m_window->get_display()->gobj()); }

        Atom get_xatom(const Glib::ustring& name) const
            { return gdk_x11_get_xatom_by_name_for_display(
                m_window->get_display()->gobj(), name.c_str()); }

        GdkDisplay* get_gdk_display() const
            { return m_window->get_display()->gobj(); }

        Glib::ustring get_utf8_property(const Glib::ustring& property) const;
        Glib::ustring get_text_property(Atom property) const;
    };

    DisplayWatch(const Glib::ustring& display_name = "");
    virtual ~DisplayWatch();
    const std::list<WindowInfo>& get_windows();

protected:
    Glib::RefPtr<Gdk::Display> m_display;
    std::list<WindowInfo> m_windows_list;

    void update_windows_list();
};

#endif // __vncbox_DISPLAY_WATCH_H
