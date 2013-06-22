/* vncboxwindow.h
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

#ifndef __vncbox_VNCBOX_WINDOW_H
#define __vncbox_VNCBOX_WINDOW_H

#include <gtkmm.h>
#include <gtkmm/socket.h>

#include "displaywatch.h"
#include "notifydata.h"
#include "vncserverinfodialog.h"
#include "vncclientsinfodialog.h"

class VncBoxWindow : public Gtk::Window
{

public:
    VncBoxWindow();
    virtual ~VncBoxWindow();

    Gtk::Socket& get_socket() { return m_socket; }

    // X server signals
    sigc::signal<void> signal_xserver_restart;
    sigc::signal<void> signal_socket_ready;
    void on_xserver_ready(unsigned short display);
    void on_xserver_closing(unsigned short display);

    // VNC server signals
    sigc::signal<void> signal_vnc_start;
    sigc::signal<void> signal_vnc_stop;
    void set_vnc_state(bool running);
    void on_vnc_server_ready(const std::string& external_ip,
        unsigned short external_rfb_port,
        unsigned short external_http_port,
        bool http_ssl,
        const std::string& password);

    // applications signals
    sigc::signal<void, Glib::ustring> signal_application_stop;
    sigc::signal<void, Glib::ustring> signal_application_start;
    void set_applications_list(const std::map<Glib::ustring, Glib::ustring>& applications); // map: name, title
    void set_application_state(const Glib::ustring& name, bool running);

    // client notify signals
    NotifyResponse on_request_accept(const NotifyData& notifydata);
    void on_notify_after_accept(const NotifyData& notifydata);
    void on_notify_gone(const NotifyData& notifydata);

    bool confirm_no_upnp();

protected:
    virtual bool on_delete_event(GdkEventAny *event);
    virtual void on_quit();
    virtual void on_map();
    virtual bool on_plug_removed();
    virtual void on_xserver_restart();
    virtual void on_vnc_activate();
    virtual void on_vnc_server_info();
    virtual void on_vnc_clients_info();
    virtual void on_windows_menu_activate();
    virtual void on_application_item_activate(const Glib::ustring& application);
    virtual void on_window_item_activate(DisplayWatch::WindowInfo winfo);
    bool confirm_quit();
    void update_windows_list();

    VncServerInfoDialog m_vnc_server_info_dialog;
    VncClientsInfoDialog m_vnc_clients_info_dialog;
    Gtk::UIManager::ui_merge_id m_applications_menu_ui_id;
    Gtk::UIManager::ui_merge_id m_windows_menu_ui_id;
    Glib::RefPtr<Gtk::UIManager> m_ui_manager;
    Glib::RefPtr<Gtk::ActionGroup> m_main_action_group;
    Glib::RefPtr<Gtk::ActionGroup> m_applications_action_group;
    Glib::RefPtr<Gtk::ActionGroup> m_windows_action_group;
    Gtk::Socket m_socket;
    Gtk::Statusbar m_statusbar;
    DisplayWatch *m_display_watch;
};

#endif // __vncbox_VNCBOX_WINDOW_H
