/* vncserverinfodialog.h
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

#ifndef __vncbox_VNCSERVERINFODIALOG_H
#define __vncbox_VNCSERVERINFODIALOG_H

#include <gtkmm.h>
#include <map>

class VncServerInfoDialog : public Gtk::Dialog
{
public:
    VncServerInfoDialog(Gtk::Window& parent);

    void set_display(int display);
    void set_rfb(const Glib::ustring& rfb_str);
    void set_http(const Glib::ustring& http_str);
    void set_password(const Glib::ustring& password_str);

protected:
    virtual void on_response(int response_id);
    virtual void uri_hook(Gtk::LinkButton* button, const Glib::ustring& uri);
    void copy_to_clipboard(const std::string& str);
    Gtk::Button* make_clipboard_button(const std::string& var);

    std::map<std::string, Glib::ustring> m_str_map;

    Gtk::Label      m_display_label;
    Gtk::Label      m_rfb_label;
    Gtk::Label      m_password_label;
    Gtk::LinkButton m_http_button;
};

#endif // __vncbox_VNCSERVERINFODIALOG_H
