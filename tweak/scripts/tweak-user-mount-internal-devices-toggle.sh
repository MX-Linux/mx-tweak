#!/bin/bash

#this script is part of mx-tweak 
#the purpose is to enable/disable user mounting of internal devices

enable()
{
    if [ -d /etc/polkit-1/localauthority/50-local.d ]; then
    echo "/etc/polkit-1/localauthority/50-local.d found"
    else
    mkdir -p /etc/polkit-1/localauthority/50-local.d
    fi
    
    cp /usr/share/mx-tweak/50-udisks.pkla /etc/polkit-1/localauthority/50-local.d/50-udisks.pkla 
    touch /etc/tweak-udisks.chk
}

disable()
{
    rm -f /etc/polkit-1/localauthority/50-local.d/50-udisks.pkla
    rm -f /etc/tweak-udisks.chk
}

main()
{
$CMD
}

CMD=$1
main

exit 0
