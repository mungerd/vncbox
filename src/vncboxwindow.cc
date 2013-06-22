/* vncboxwindow.cc
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

#include "vncboxwindow.h"
#include "displaywatch.h"

#include <iostream>

using Glib::ustring;

VncBoxWindow::VncBoxWindow()
    :
    m_vnc_server_info_dialog(*this),
    m_vnc_clients_info_dialog(*this),
    m_applications_menu_ui_id(0),
    m_windows_menu_ui_id(0)
{
    m_display_watch = 0;

    set_title("VNC Box");
    set_border_width(10);

    Gtk::VBox *vbox = manage(new Gtk::VBox());
    add(*vbox);

    m_main_action_group = Gtk::ActionGroup::create();
    m_applications_action_group = Gtk::ActionGroup::create();
    m_windows_action_group = Gtk::ActionGroup::create();

    m_main_action_group->add(Gtk::Action::create("XServerMenu", _("_X Server")));
    m_main_action_group->add(Gtk::Action::create("XServerRestart", Gtk::Stock::REFRESH, _("_Restart")),
        Gtk::AccelKey("<control>R"),
        sigc::mem_fun(*this, &VncBoxWindow::on_xserver_restart));
    m_main_action_group->add(Gtk::Action::create("Quit", Gtk::Stock::QUIT),
        sigc::mem_fun(*this, &VncBoxWindow::on_quit));

    m_main_action_group->add(Gtk::Action::create("VncMenu", _("_VNC Server")));
    m_main_action_group->add(Gtk::ToggleAction::create("VncActive", _("_Active")),
        sigc::mem_fun(*this, &VncBoxWindow::on_vnc_activate));
    m_main_action_group->add(Gtk::Action::create("VncServerInfo", _("_Server Information")),
        sigc::mem_fun(*this, &VncBoxWindow::on_vnc_server_info));
    m_main_action_group->add(Gtk::Action::create("VncClientsInfo", _("_Clients Information")),
        sigc::mem_fun(*this, &VncBoxWindow::on_vnc_clients_info));

    m_main_action_group->add(Gtk::Action::create("ApplicationsMenu", _("_Applications")));
    m_main_action_group->add(Gtk::Action::create("WindowsMenu", _("_Windows")),
        sigc::mem_fun(*this, &VncBoxWindow::on_windows_menu_activate));

    m_ui_manager = Gtk::UIManager::create();
    m_ui_manager->insert_action_group(m_main_action_group);
    m_ui_manager->insert_action_group(m_applications_action_group);
    m_ui_manager->insert_action_group(m_windows_action_group);

    ustring ui_info =
         "<ui>"
        "  <menubar name='MainMenu'>"
        "    <menu name='XServerMenu' action='XServerMenu'>"
        "      <menuitem name='XServerRestart' action='XServerRestart' />"
        "      <separator />"
        "      <menuitem name='Quit' action='Quit' />"
        "    </menu>"
        "    <menu name='VncMenu' action='VncMenu'>"
        "      <menuitem name='VncActive' action='VncActive' />"
        "      <separator />"
        "      <menuitem name='VncServerInfo' action='VncServerInfo' />"
        "      <menuitem name='VncClientsInfo' action='VncClientsInfo' />"
        "    </menu>"
        "    <menu name='ApplicationsMenu' action='ApplicationsMenu'>"
        "      <placeholder name='ApplicationsList' />"
        "    </menu>"
        "    <menu name='WindowsMenu' action='WindowsMenu'>"
        "      <placeholder name='WindowsList' />"
        "    </menu>"
        "  </menubar>"
        "  <toolbar name='MainToolbar' action='MainMenuBar'>"
        "    <placeholder name='ToolItems'>"
        "      <toolitem name='XServerRestart' action='XServerRestart'/>"
        "      <separator/>"
        "      <toolitem name='Quit' action='Quit'/>"
        "      <separator/>"
        "    </placeholder>"
        "  </toolbar>"
        "</ui>";

    try {
        m_ui_manager->add_ui_from_string(ui_info);

        vbox->pack_start(*m_ui_manager->get_widget("/MainMenu"), Gtk::PACK_SHRINK);
        vbox->pack_start(*m_ui_manager->get_widget("/MainToolbar"), Gtk::PACK_SHRINK);

        add_accel_group(m_ui_manager->get_accel_group());

    }
    catch (const Glib::Error& e) {
        std::cerr << "error building UI: " <<  e.what();
    }

    vbox->pack_start(m_socket);
    m_socket.signal_plug_removed().connect(sigc::mem_fun(*this, &VncBoxWindow::on_plug_removed));
    vbox->pack_start(m_statusbar, Gtk::PACK_SHRINK);

    m_socket.property_can_focus() = true;
    m_socket.property_has_focus() = true;

    set_vnc_state(false);

    show_all_children();
}

VncBoxWindow::~VncBoxWindow()
{
}

bool
VncBoxWindow::confirm_no_upnp ()
{
    Gtk::MessageDialog dialog(*this, _("UPnP is unavailable"),
        false,
        Gtk::MESSAGE_QUESTION,
        Gtk::BUTTONS_OK_CANCEL);
    dialog.set_secondary_text(_("UPnP will be ignored."));

    int result = dialog.run();

    return result == Gtk::RESPONSE_OK;
}

bool
VncBoxWindow::confirm_quit ()
{
    Gtk::MessageDialog dialog(*this, _("Quitting."),
        false,
        Gtk::MESSAGE_QUESTION,
        Gtk::BUTTONS_OK_CANCEL);
    dialog.set_secondary_text(_("Current session state will be lost."));

    int result = dialog.run();

    return result == Gtk::RESPONSE_OK;
}

void
VncBoxWindow::on_quit()
{
	if (confirm_quit())
		hide();
}

bool
VncBoxWindow::on_delete_event(GdkEventAny *event)
{
    return confirm_quit() ? false : true;
}

void
VncBoxWindow::on_map()
{
    Gtk::Window::on_map();

    signal_socket_ready();
}

bool
VncBoxWindow::on_plug_removed()
{
    // return true to avoid destroying the socket
    return true;
}

void
VncBoxWindow::on_windows_menu_activate()
{
    update_windows_list();
}

void
VncBoxWindow::set_applications_list(const std::map<ustring,ustring>& applications)
{
    // clean up UI
    if (m_applications_menu_ui_id) {
        // clear applications list
        m_ui_manager->remove_ui(m_applications_menu_ui_id);
    }
    m_applications_menu_ui_id = m_ui_manager->new_merge_id();

    // clean up action group
    m_ui_manager->remove_action_group(m_applications_action_group);
    m_applications_action_group = Gtk::ActionGroup::create();
    m_ui_manager->insert_action_group(m_applications_action_group);

    // new list
    for (std::map<ustring,ustring>::const_iterator it = applications.begin();
        it != applications.end(); it++) {

        ustring action_name = "Application_" + it->first;

        Glib::RefPtr<Gtk::ToggleAction> action =
            Gtk::ToggleAction::create(action_name,
                it->second[0] == '!' ? "<span color='#a00'>" + it->second.substr(1) + "</span>" :
                it->second);

        m_applications_action_group->add(action, sigc::bind(sigc::mem_fun(*this,
            &VncBoxWindow::on_application_item_activate), it->first));

        m_ui_manager->add_ui(m_applications_menu_ui_id,
            "/MainMenu/ApplicationsMenu/ApplicationsList",
            action_name, action_name,
            Gtk::UI_MANAGER_MENUITEM,
            false); // insert at bottom

        // set markup
        Gtk::MenuItem* menu_item = static_cast<Gtk::MenuItem*>(
            m_ui_manager->get_widget("/MainMenu/ApplicationsMenu/ApplicationsList/" + action_name));
        if (menu_item) {
            Gtk::Label* label = static_cast<Gtk::Label*>(menu_item->get_child());
            if (label) label->set_use_markup(true);
        }
    }

    if (applications.empty()) {
        ustring action_name = _("No applications");
        Glib::RefPtr<Gtk::Action> action =
            Gtk::Action::create(action_name, action_name);
        m_applications_action_group->add(action);
        m_ui_manager->add_ui(m_windows_menu_ui_id,
            "/MainMenu/ApplicationsMenu/ApplicationsList",
            action_name, action_name,
            Gtk::UI_MANAGER_MENUITEM);
    }
}

void
VncBoxWindow::update_windows_list()
{
    // clean up UI
    if (m_windows_menu_ui_id) {
        // clear windows list
        m_ui_manager->remove_ui(m_windows_menu_ui_id);
    }
    m_windows_menu_ui_id = m_ui_manager->new_merge_id();

    const std::list<DisplayWatch::WindowInfo>& winfos = m_display_watch->get_windows();

    // clean up action group
    m_ui_manager->remove_action_group(m_windows_action_group);
    m_windows_action_group = Gtk::ActionGroup::create();
    m_ui_manager->insert_action_group(m_windows_action_group);

    // new list
    int iwindow = 0;
    for (std::list<DisplayWatch::WindowInfo>::const_iterator it = winfos.begin();
        it != winfos.end(); it++) {

        ustring action_name = "ActivateWindow" + ustring::format(++iwindow);

        Glib::RefPtr<Gtk::Action> action =
            Gtk::Action::create(action_name, it->get_icon_name());

        m_windows_action_group->add(action, sigc::bind(sigc::mem_fun(*this,
            &VncBoxWindow::on_window_item_activate), *it));

        m_ui_manager->add_ui(m_windows_menu_ui_id,
            "/MainMenu/WindowsMenu/WindowsList",
            action_name, action_name,
            Gtk::UI_MANAGER_MENUITEM);
    }

    if (winfos.empty()) {
        ustring action_name = _("No windows");
        Glib::RefPtr<Gtk::Action> action =
            Gtk::Action::create(action_name, action_name);
        m_windows_action_group->add(action);
        m_ui_manager->add_ui(m_windows_menu_ui_id,
            "/MainMenu/WindowsMenu/WindowsList",
            action_name, action_name,
            Gtk::UI_MANAGER_MENUITEM);
    }
}

void
VncBoxWindow::on_xserver_restart()
{
    signal_xserver_restart();
}

void
VncBoxWindow::on_application_item_activate(const ustring& application)
{
    Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(
        m_applications_action_group->get_action("Application_" + application));

    if (action->get_active())
        signal_application_start(application);
    else
        signal_application_stop(application);
}

void
VncBoxWindow::set_application_state(const ustring& application, bool running)
{
    Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(
        m_applications_action_group->get_action("Application_" + application));

    if (action->get_active() != running)
        action->set_active(running);
}

void
VncBoxWindow::on_window_item_activate(DisplayWatch::WindowInfo winfo)
{
    winfo.activate();
}

void
VncBoxWindow::on_xserver_ready(unsigned short display)
{
    if (!m_display_watch)
        m_display_watch = new DisplayWatch(ustring::compose(":%1", display));

    update_windows_list();

    m_vnc_server_info_dialog.set_display(display);
}

void
VncBoxWindow::on_xserver_closing(unsigned short display)
{
    delete m_display_watch;
    m_display_watch = 0;
}

void
VncBoxWindow::on_vnc_activate()
{
    Glib::RefPtr<Gtk::ToggleAction> action = Glib::RefPtr<Gtk::ToggleAction>::cast_static(
        m_main_action_group->get_action("VncActive"));

    set_vnc_state(action->get_active());

    if (action->get_active())
        signal_vnc_start();
    else
        signal_vnc_stop();
}

void
VncBoxWindow::set_vnc_state(bool running)
{
    Glib::RefPtr<Gtk::ToggleAction> vnc_active_action =
        Glib::RefPtr<Gtk::ToggleAction>::cast_static(
        m_main_action_group->get_action("VncActive"));

    if (vnc_active_action->get_active() != running)
        vnc_active_action->set_active(running);

    Glib::RefPtr<Gtk::Action> vnc_server_info_action =
        Glib::RefPtr<Gtk::Action>::cast_static(
            m_main_action_group->get_action("VncServerInfo"));
    Glib::RefPtr<Gtk::Action> vnc_clients_info_action =
        Glib::RefPtr<Gtk::Action>::cast_static(
            m_main_action_group->get_action("VncClientsInfo"));

    vnc_server_info_action->set_sensitive(running);
    vnc_clients_info_action->set_sensitive(running);

    m_vnc_clients_info_dialog.clear_clients();

    m_statusbar.pop();
    if (running)
        m_statusbar.push(_("VNC server ready."));
    else
        m_statusbar.push(_("VNC server stopped."));
}

NotifyResponse
VncBoxWindow::on_request_accept(const NotifyData& notifydata)
{
    Gtk::MessageDialog dialog(*this,
        "Incoming client",
        false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_NONE);

    dialog.set_secondary_text(
        ustring::compose("ID: %1\n", notifydata.client_id) +
        ustring::compose("IP: %1\n", notifydata.client_ip));

    dialog.add_button(Gtk::Stock::CANCEL,   Gtk::RESPONSE_REJECT);
    dialog.add_button("View only",          Gtk::RESPONSE_OK);
    if (notifydata.login_viewonly != 1)
        dialog.add_button(Gtk::Stock::CONNECT,  Gtk::RESPONSE_ACCEPT);

    dialog.set_default_response(Gtk::RESPONSE_REJECT);

    int result = dialog.run();

    // Gtk::RESPONSE_DELETE_EVENT --> hide

    return
        result == Gtk::RESPONSE_ACCEPT ? NOTIFY_RESPONSE_ACCEPT :
        result == Gtk::RESPONSE_OK ? NOTIFY_RESPONSE_VIEWONLY :
        NOTIFY_RESPONSE_DENY;
}

void
VncBoxWindow::on_notify_after_accept(const NotifyData& notifydata)
{
    m_vnc_clients_info_dialog.add_client(notifydata);

    m_statusbar.pop();
    m_statusbar.push(ustring::compose(_("VNC client connected (ID: %1 / IP: %2)."),
        notifydata.client_id, notifydata.client_ip));
}

void
VncBoxWindow::on_notify_gone(const NotifyData& notifydata)
{
    m_vnc_clients_info_dialog.remove_client(notifydata.client_id);

    m_statusbar.pop();
    m_statusbar.push(ustring::compose(_("VNC client disconnected (ID: %1 / IP: %2)."),
        notifydata.client_id, notifydata.client_ip));
}

void
VncBoxWindow::on_vnc_server_ready(const std::string& external_ip,
        unsigned short external_rfb_port,
        unsigned short external_http_port,
        bool http_ssl,
        const std::string& password)

{
    set_vnc_state(true);

    m_vnc_server_info_dialog.set_rfb(ustring::compose("%1:%2",
        external_ip, external_rfb_port));

    if (external_http_port)
        m_vnc_server_info_dialog.set_http(ustring::compose("%1://%2:%3",
            http_ssl ? "https" : "http", external_ip, external_http_port));
    else
        m_vnc_server_info_dialog.set_http("");

    m_vnc_server_info_dialog.set_password(password);

    m_vnc_server_info_dialog.show();
}

void
VncBoxWindow::on_vnc_server_info()
{
    m_vnc_server_info_dialog.show();
}

void
VncBoxWindow::on_vnc_clients_info()
{
    m_vnc_clients_info_dialog.show();
}
