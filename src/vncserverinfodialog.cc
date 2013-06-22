/* vncserverinfodialog.cc
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

#include <glibmm/i18n.h>
#include <gtkmm.h>

#include "vncserverinfodialog.h"


using Glib::ustring;


VncServerInfoDialog::VncServerInfoDialog(Gtk::Window& parent)
    :
    Gtk::Dialog(_("VNC Server Information"), parent)
{
#if 0
    get_window()->set_skip_taskbar_hint(true);
#endif
    set_resizable(false);

    Gtk::Table* table = manage(new Gtk::Table(4, 3));
    get_vbox()->pack_start(*table);

    table->set_spacings(5);

    table->attach(*manage(new Gtk::Label(_("Display:"), 1.0, 0.5)), 0, 1, 0, 1);
    table->attach(m_display_label, 1, 2, 0, 1);
    table->attach(*manage(new Gtk::Label(_("RFB:"), 1.0, 0.5)), 0, 1, 1, 2);
    table->attach(m_rfb_label, 1, 2, 1, 2);
    table->attach(*manage(new Gtk::Label(_("HTTP:"), 1.0, 0.5)), 0, 1, 2, 3);
    table->attach(m_http_button, 1, 2, 2, 3);
    table->attach(*manage(new Gtk::Label(_("Password:"), 1.0, 0.5)), 0, 1, 3, 4);
    table->attach(m_password_label, 1, 2, 3, 4);

    m_display_label.set_alignment(0.0, 0.5);
    m_rfb_label.set_alignment(0.0, 0.5);
    m_http_button.set_alignment(0.0, 0.5);
    m_password_label.set_alignment(0.0, 0.5);

    table->attach(*make_clipboard_button("display"),    2, 3, 0, 1);
    table->attach(*make_clipboard_button("rfb"),        2, 3, 1, 2);
    table->attach(*make_clipboard_button("http"),       2, 3, 2, 3);
    table->attach(*make_clipboard_button("password"),   2, 3, 3, 4);

    m_http_button.set_uri_hook(sigc::mem_fun(*this, &VncServerInfoDialog::uri_hook));

    m_display_label.set_selectable();
    m_rfb_label.set_selectable();
    m_password_label.set_selectable();

    set_border_width(5);
    get_vbox()->set_border_width(5);
    get_vbox()->set_spacing(14);
    get_action_area()->set_spacing(6);

    add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_OK);

    get_vbox()->show_all_children();

    m_str_map["display"] = "";
    m_str_map["rfb"] = "";
    m_str_map["http"] = "";
    m_str_map["password"] = "";
}

void
VncServerInfoDialog::on_response(int response_id)
{
    hide();
}

void
VncServerInfoDialog::set_display(int display)
{
    m_str_map["display"] = ustring::compose("%1:%2", g_get_host_name(), display);
    m_display_label.set_markup("<span weight='bold'>" + m_str_map["display"] + "</span>");
}

void
VncServerInfoDialog::set_rfb(const ustring& rfb_str)
{
    m_str_map["rfb"] = rfb_str;
    m_rfb_label.set_markup("<span weight='bold'>" + rfb_str + "</span>");
}

void
VncServerInfoDialog::set_http(const ustring& http_str)
{
    m_str_map["http"] = http_str;
    m_http_button.set_label(http_str);
    m_http_button.set_uri(http_str);

    if (http_str.empty())
        m_http_button.hide();
    else
        m_http_button.show();
}

void
VncServerInfoDialog::set_password(const ustring& password_str)
{
    m_str_map["password"] = password_str;
    m_password_label.set_markup("<span weight='bold'>" + password_str + "</span>");
}

void
VncServerInfoDialog::uri_hook(Gtk::LinkButton* button, const Glib::ustring& uri)
{
    Glib::spawn_command_line_async("gnome-open " + uri);
}

void
VncServerInfoDialog::copy_to_clipboard(const std::string& var)
{
    Glib::RefPtr<Gtk::Clipboard> clipboard = Gtk::Clipboard::get();
    clipboard->set_text(m_str_map[var]);
    clipboard->store();
}

Gtk::Button*
VncServerInfoDialog::make_clipboard_button(const std::string& var)
{
    Gtk::Button* button;
    button = manage(new Gtk::Button(_("Copy to clipboard")));
    button->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this,
        &VncServerInfoDialog::copy_to_clipboard), var));
    return button;
}
