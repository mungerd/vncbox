[X server]

width  = 640
height = 480

xserver options = -noxv -host-cursor -keybd ephyr,,xkblayout="us"

preferred display = 10

# applications available to run as self
self applications = gnome-terminal;xlogo

# applications available to run setuid
anonymous applications = metacity;xournal

startup applications = metacity;xournal



[VNC]

RFB port    = RANDOM
RFB SSL     = true
SSL cert    = SAVE

HTTP        = true
HTTP port   = RANDOM
HTTP SSL    = false

password    = RANDOM
UPnP        = true

# misc options
options     = -viewonly -forever


# applications to be run as self must be defined in the profile

[application gnome-terminal]
command = gnome-terminal
title = GNOME Terminal

[application metacity]
title = Metacity Window Manager

[application xournal]
title = Xournal

[application xlogo]
command = /usr/bin/xlogo
title = xlogo
