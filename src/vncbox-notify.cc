// g++ -Wall -o dbus-client `pkg-config dbus-c++-1 --cflags --libs` dbus-client.cc && ./dbus-client

#include <dbus-c++/dbus.h>
#include "vncbox-client-glue.h"
#include "notifydata.h"

#include <iostream>
#include <string>
#include <cstdlib>


static const char *VNCBOX_SERVICE_NAME = "org.gnome.VncBox";
static const char *VNCBOX_SERVICE_PATH = "/org/gnome/VncBox";

class VncBoxClient
: public org::gnome::VncBox_proxy,
  public DBus::IntrospectableProxy,
  public DBus::ObjectProxy
{
public:
	VncBoxClient(DBus::Connection &connection);
};

VncBoxClient::VncBoxClient(DBus::Connection &connection)
    : ObjectProxy(connection, VNCBOX_SERVICE_PATH, VNCBOX_SERVICE_NAME)
{
}


int
main(int argc, char** argv)
{
    try {
        DBus::BusDispatcher dispatcher;
    	DBus::default_dispatcher = &dispatcher;

	    DBus::Connection dbus_connection = DBus::Connection::SessionBus();

        VncBoxClient client(dbus_connection);

        std::string mode = std::getenv("RFB_MODE");

        if (mode == "accept") {
            return client.RequestAccept(NotifyData::env_to_map());
        }
        else if (mode == "afteraccept") {
            client.NotifyAfterAccept(NotifyData::env_to_map());
            return 0;
        }
        else if (mode == "gone") {
            client.NotifyGone(NotifyData::env_to_map());
            return 0;
        }
        else {
            std::cerr << "unknown RFB mode `" << mode << "'" << std::endl;
        }
    }
    catch (DBus::Error& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 127;
}
