/* displaywatch.cc
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

using Glib::ustring;

DisplayWatch::DisplayWatch(const Glib::ustring& display_name)
    : m_display(0)
{
    if (display_name.empty())
        m_display = Gdk::Display::get_default();
    else
        m_display = Gdk::Display::open(display_name);

    if (m_display == 0)
        throw DisplayWatchError(DisplayWatchError::INVALID_DISPLAY_NAME,
            display_name.empty() ? "cannot open default display"
            : "cannot open display " + display_name);
}

DisplayWatch::~DisplayWatch()
{
    m_display->close();
}

const std::list<DisplayWatch::WindowInfo>&
DisplayWatch::get_windows()
{
    update_windows_list();
    return m_windows_list;
}

void
DisplayWatch::update_windows_list()
{
    m_windows_list.clear();

    std::list<Glib::RefPtr<Gdk::Window> > windows =
        m_display->get_default_screen()->get_window_stack();

    for (std::list<Glib::RefPtr<Gdk::Window> >::iterator it = windows.begin();
        it != windows.end(); it++)
        m_windows_list.push_back(*it);
}
