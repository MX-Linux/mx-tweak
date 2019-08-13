#!/bin/bash
##this file is part of MX Compton Manager, created for the MX Linux project
##by dolphin oracle, January 2017.  
##code is gpl3 licensed.

#launch compton, with user config file

#deactivate xfce4 compositor if its running
if [ $(xfconf-query -c xfwm4 -p /general/use_compositing) ]; then
    xfconf-query -c xfwm4 -p /general/use_compositing -s false
fi

#lauch compton with default config file
compton --dbus --config /home/$USER/.config/compton.conf -b

exit 0
