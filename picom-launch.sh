#!/bin/bash
##this file is part of MX Tweak Manager, created for the MX Linux project
##by dolphin oracle, May 2025.  

#launch picom, with user config file

#deactivate xfce4 compositor if its running
if [ $(xfconf-query -c xfwm4 -p /general/use_compositing) ]; then
    xfconf-query -c xfwm4 -p /general/use_compositing -s false
fi

#lauch compton with default config file
picom --dbus --config /home/$USER/.config/picom.conf -b

exit 0
