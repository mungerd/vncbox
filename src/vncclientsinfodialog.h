/* vncclientsinfodialog.h
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

#ifndef __vncbox_VNCCLIENTSINFODIALOG_H
#define __vncbox_VNCCLIENTSINFODIALOG_H

#include <gtkmm.h>

#include "notifydata.h"

class VncClientsInfoDialog : public Gtk::Dialog
{
public:
    VncClientsInfoDialog(Gtk::Window& parent);

    void add_client(const NotifyData& notifydata);
    void remove_client(long client_id);
    void clear_clients();

protected:
    virtual void on_response(int response_id);

    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:

        ModelColumns()
            { add(client_id); add(username); add(client_ip); add(viewonly); add(login_time);}

        Gtk::TreeModelColumn<long> client_id;
        Gtk::TreeModelColumn<Glib::ustring> username;
        Gtk::TreeModelColumn<Glib::ustring> client_ip;
        Gtk::TreeModelColumn<Glib::ustring> viewonly;
        Gtk::TreeModelColumn<Glib::ustring> login_time;
    };

    ModelColumns m_columns;
    Gtk::TreeView m_tree_view;
    Glib::RefPtr<Gtk::ListStore> m_list_store;
};

#endif // __vncbox_VNCCLIENTSINFODIALOG_H
