# VNC Box

Securely grant remote users access to a virtual desktop that runs in a window on
your machine.


## Why VNC Box?

This is useful if you want to do remote teaching, where you need to:

* stream a screencast of you using an application;

* give remote users access to applications running on your machine.

The key feature is: you can do both **without a risk of leaking any personal
data**, because the applications are run from within a secure environement, i.e.
a *jail*.


## How It Works

VNC Box runs the [http://www.karlrunge.com/x11vnc/](x11vnc) VNC server on the nested X server [http://freedesktop.org/wiki/Software/Xephyr](Xephyr) and runs applications in a jail as an unprivileged user.

To set up the jail, using [http://olivier.sessink.nl/jailkit/](Jailkit) is suggested.

VNC Box is written in C++ using [http://www.gtkmm.org/](gtkmm).


## Features

* Improved security of the VNC service by launching applications in a jail.
* Automatic creation of Xauthority entries for both the current user and the jail user.
* Automatic creation of UPnP port mappings for the RFB and HTTP services.
* Notification of incoming VNC clients.
* Activation of the windows of nested application via a menu to get around conflicting key mappings between the host and nested window managers.
* Possibility to create different profiles for different uses.
* Optional selection of random ports for HTTP and RFB services.
* On-the-fly random password generation.


## Examples of Practical Uses

* Give view-only access to a Xournal session that serves as a blackboard while connected with Skype to your student.
* Allow your student to write on your blackboard.


## Dependencies

### Programs
* [http://www.karlrunge.com/x11vnc/](x11vnc)
* [http://freedesktop.org/wiki/Software/Xephyr](Xephyr) (part of the X.org distribution)
* [http://olivier.sessink.nl/jailkit/](Jailkit) (suggested)

### Libraries
* [http://ftp.gnome.org/pub/GNOME/sources/gnome-common/](gnome-common)
* [http://www.gtkmm.org/](gtkmm 2.24)
* [http://freedesktop.org/wiki/Software/dbus-c%2B%2B](dbus-c++)
* [http://www.gupnp.org/](gUPnP)


## Building

Local install:

    ./autogen.sh --prefix=$PWD/install && make && make install

System-wide install:

    ./autogen.sh \
	--prefix=/usr \
	--sysconfdir=/etc \
	--libexecdir=/usr/lib/vncbox \
	--localstatedir=/var \
	--disable-static &&
    make && make install


## Configuration

Replace `VNC_USER` and `USER` in `etc/vncbox.conf` with the user names under which the VNC server and the hosted applications, respectively, must be run.  Edit this file to configure more users and applications.

Copy and edit `data/example.profile` to `~/.config/vncbox/`.  For example:

    mkdir -p ~/.config/vncbox
    cp data/example.profile ~/.config/vncbox/default.profile

Edit the profile to configure which applications should be run.

Execute vncbox with your new profile with:

    bin/vncbox default

Or, replace `default` with the name you chose for the new profile.


# Implementation Notes


## Nested X server

The nested X server (Xephyr) is launched as a GtkPlug plugged into a GtkSocket in the main VNC Box window.

Before starting the X server, VNC Box creates Xauth entries (using [http://github.com/mungerd/xauthxx/](xauthxx) for the current user and the jail user, for the new display. This is implemented in the `XServer` object.


## Jail

The default VNC Box setup assumes a jail is configured (see `scripts/create_vncjail-archlinux`) for user `vnc` whose login shell is a chroot shell (like `jk_chrootsh`) pointing to the jail.

The `vncbox-notify` program must also be installed in the jail.


## Client Applications

Applications can be started whether as owned by the current user or as owned by the jail user. For that purpose, the `vncbox-exec` program must be installed setuid.


## VNC Server

The VNC server (x11vnc) is owned by the jail user. Upon an incoming connection or a lost connection of a VNC client, the `vncbox-notify` program is launched. It communicates via D-Bus with the VNC Box program. For an incoming connection, `vncbox-notify` asks VNC Box if the new VNC client should be allowed to connect to the VNC service. For a lost connection, `vncbox-notify` simply notifies VNC Box of the disconnected client.

VNC Box, owned by the current user, starts a private dbus-daemon listening on an abstract socket in order to allow `vncbox-notify`, owned by the jail user, to communicate with VNC Box.

For security purposes, applications allowed to be run setuid must be listed in the system-wide configuration file `/etc/vncbox.conf`.


## UPnP

If requested, the `VncServer` object can also create UPnP port mappings for the RFB and HTTP services.
