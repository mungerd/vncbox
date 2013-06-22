/* vncclientsinfodialog.cc
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
#include <time.h>

#include "vncclientsinfodialog.h"


VncClientsInfoDialog::VncClientsInfoDialog(Gtk::Window& parent)
    : Gtk::Dialog(_("VNC Clients Information"), parent)
{
#if 0
    get_window()->set_skip_taskbar_hint(true);
#endif
    set_resizable(true);

    Gtk::ScrolledWindow* scrolled_window = manage(new Gtk::ScrolledWindow());
    get_vbox()->pack_start(*scrolled_window);
    scrolled_window->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolled_window->set_size_request(450, 200);

    scrolled_window->add(m_tree_view);
    m_list_store = Gtk::ListStore::create(m_columns);
    m_tree_view.set_model(m_list_store);
    m_tree_view.append_column(_("ID"),          m_columns.client_id);
    m_tree_view.append_column(_("Username"),    m_columns.username);
    m_tree_view.append_column(_("IP address"),  m_columns.client_ip);
    m_tree_view.append_column(_("View only"),   m_columns.viewonly);
    m_tree_view.append_column(_("Login time"),  m_columns.login_time);

    set_border_width(5);
    get_vbox()->set_border_width(5);
    get_vbox()->set_spacing(14);
    get_action_area()->set_spacing(6);

    add_button(Gtk::Stock::CLOSE, Gtk::RESPONSE_OK);

    get_vbox()->show_all_children();
}

void
VncClientsInfoDialog::on_response(int response_id)
{
    hide();
}

void
VncClientsInfoDialog::add_client(const NotifyData& notifydata)
{
    struct tm time_tm;
    localtime_r(&notifydata.login_time, &time_tm);
    char time_str[127];
    strftime(time_str, sizeof(time_str), "%F %H:%M:%S", &time_tm);

    Gtk::TreeModel::Row row = *m_list_store->append();
    row[m_columns.client_id]    = notifydata.client_id;
    row[m_columns.username]     = notifydata.username;
    row[m_columns.client_ip]    = notifydata.client_ip;
    row[m_columns.login_time]   = time_str;

    switch (notifydata.login_viewonly) {
    case 0:
        row[m_columns.viewonly] = _("No");
        break;
    case 1:
        row[m_columns.viewonly] = _("Yes");
        break;
    default:
        row[m_columns.viewonly] = "";
    }
}

void
VncClientsInfoDialog::remove_client(long client_id)
{
    for (Gtk::TreeModel::iterator it = m_list_store->get_iter("0"); it; it++) {
        Gtk::TreeModel::Row row = *it;
        if (row[m_columns.client_id]) {
            m_list_store->erase(it);
            return;
        }
    }
}

void
VncClientsInfoDialog::clear_clients()
{
    m_list_store->clear();
}
