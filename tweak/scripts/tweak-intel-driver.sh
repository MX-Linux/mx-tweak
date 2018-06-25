#!/bin/bash

#this script is part of mx-tweak 
#the purpose is to enable/disable intel driver override

enable()
{
    cp /usr/share/mx-tweak/20-intel.conf /etc/X11/xorg.conf.d/20-intel.conf
}

disable()
{
    rm /etc/X11/xorg.conf.d/20-intel.conf
}

main()
{
$CMD
}

CMD=$1
main

exit 0
